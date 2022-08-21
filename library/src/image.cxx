#include "image.h"

#include <vtkImageDifference.h>
#include <vtkImageExport.h>
#include <vtkImageImport.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkPNGWriter.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

#include <cassert>
#include <vector>

namespace f3d
{
class image::internals
{
public:
  std::vector<unsigned char> Buffer;
  unsigned int Width = 0;
  unsigned int Height = 0;
  unsigned int Channels = 0;

  void UpdateBufferSize() { this->Buffer.resize(this->Width * this->Height * this->Channels); }

  vtkSmartPointer<vtkImageImport> GetVTKImporter() const
  {
    assert(this->Buffer.size() == this->Width * this->Height * this->Channels);

    vtkNew<vtkImageImport> importer;
    importer->CopyImportVoidPointer(
      const_cast<unsigned char*>(this->Buffer.data()), this->Buffer.size());
    importer->SetNumberOfScalarComponents(this->Channels);
    importer->SetDataScalarTypeToUnsignedChar();
    importer->SetWholeExtent(0, this->Width - 1, 0, this->Height - 1, 0, 0);
    importer->SetDataExtentToWholeExtent();
    return importer;
  }

  void SetFromVTK(vtkAlgorithm* alg)
  {
    vtkNew<vtkImageExport> exporter;
    exporter->SetInputConnection(alg->GetOutputPort());
    exporter->ImageLowerLeftOn();

    int* dims = exporter->GetDataDimensions();
    this->Width = dims[0];
    this->Height = dims[1];
    this->Channels = exporter->GetDataNumberOfScalarComponents();
    this->UpdateBufferSize();

    exporter->Export(this->Buffer.data());
  }
};

//----------------------------------------------------------------------------
image::image()
  : Internals(new image::internals())
{
}

//----------------------------------------------------------------------------
image::image(const std::string& path)
  : Internals(new image::internals())
{
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);
  if (!vtksys::SystemTools::FileExists(path))
  {
    throw read_exception("Cannot open image " + path);
  }

  auto reader = vtkSmartPointer<vtkImageReader2>::Take(
    vtkImageReader2Factory::CreateImageReader2(path.c_str()));

  if (reader)
  {
    reader->SetFileName(path.c_str());
    this->Internals->SetFromVTK(reader);
  }
}

//----------------------------------------------------------------------------
image::~image()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
image::image(const image& img)
  : Internals(new image::internals())
{
  *this->Internals = *img.Internals;
}

//----------------------------------------------------------------------------
image& image::operator=(const image& img) noexcept
{
  *this->Internals = *img.Internals;
  return *this;
}

//----------------------------------------------------------------------------
image::image(image&& img) noexcept : Internals(nullptr)
{
  std::swap(this->Internals, img.Internals);
}

//----------------------------------------------------------------------------
image& image::operator=(image&& img) noexcept
{
  std::swap(this->Internals, img.Internals);
  return *this;
}

//----------------------------------------------------------------------------
unsigned int image::getWidth() const
{
  return this->Internals->Width;
}

//----------------------------------------------------------------------------
unsigned int image::getHeight() const
{
  return this->Internals->Height;
}

//----------------------------------------------------------------------------
image& image::setResolution(unsigned int width, unsigned int height)
{
  this->Internals->Width = width;
  this->Internals->Height = height;
  this->Internals->UpdateBufferSize();
  return *this;
}

//----------------------------------------------------------------------------
unsigned int image::getChannelCount() const
{
  return this->Internals->Channels;
}

//----------------------------------------------------------------------------
image& image::setChannelCount(unsigned int dim)
{
  this->Internals->Channels = dim;
  this->Internals->UpdateBufferSize();
  return *this;
}

//----------------------------------------------------------------------------
image& image::setData(unsigned char* buffer)
{
  std::copy(buffer, buffer + this->Internals->Buffer.size(), this->Internals->Buffer.begin());
  return *this;
}

//----------------------------------------------------------------------------
unsigned char* image::getData() const
{
  return this->Internals->Buffer.data();
}

//----------------------------------------------------------------------------
bool image::compare(const image& reference, double threshold, image& diff, double& error) const
{
  auto importerThis = this->Internals->GetVTKImporter();
  auto importerRef = reference.Internals->GetVTKImporter();

  vtkNew<vtkImageDifference> imDiff;
  // handle threshold outside of vtkImageDifference:
  // https://gitlab.kitware.com/vtk/vtk/-/issues/18152
  imDiff->SetThreshold(0);
  imDiff->SetInputConnection(importerThis->GetOutputPort());
  imDiff->SetImageConnection(importerRef->GetOutputPort());
  imDiff->UpdateInformation();
  error = imDiff->GetThresholdedError();

  if (error <= threshold)
  {
    imDiff->Update();
    error = imDiff->GetThresholdedError();
  }

  if (error <= threshold)
  {
    return true;
  }
  else
  {
    diff.Internals->SetFromVTK(imDiff);
    return false;
  }
}

//----------------------------------------------------------------------------
bool image::operator==(const image& reference) const
{
  f3d::image diff;
  double error;
  return this->compare(reference, 0, diff, error);
}

//----------------------------------------------------------------------------
bool image::operator!=(const image& reference) const
{
  return !this->operator==(reference);
}

//----------------------------------------------------------------------------
void image::save(const std::string& path) const
{
  auto importer = this->Internals->GetVTKImporter();

  vtkNew<vtkPNGWriter> writer;
  writer->SetFileName(path.c_str());
  writer->SetInputConnection(importer->GetOutputPort());
  writer->Write();

  if (writer->GetErrorCode() != 0)
  {
    throw write_exception("Cannot write " + path);
  }
}

//----------------------------------------------------------------------------
image::write_exception::write_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
image::read_exception::read_exception(const std::string& what)
  : exception(what)
{
}

}
