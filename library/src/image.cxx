#include "image.h"

#include "init.h"

#include <vtkBMPWriter.h>
#include <vtkImageData.h>
#include <vtkImageDifference.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkJPEGWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFWriter.h>
#include <vtksys/SystemTools.hxx>

#include <cassert>
#include <vector>

namespace f3d
{
class image::internals
{
public:
  vtkSmartPointer<vtkImageData> Image;
};

//----------------------------------------------------------------------------
image::image()
  : Internals(new image::internals())
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
}

//----------------------------------------------------------------------------
image::image(unsigned int width, unsigned int height, unsigned int channelCount, ChannelType type)
  : Internals(new image::internals())
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
  this->Internals->Image->SetDimensions(width, height, 1);

  switch (type)
  {
    case ChannelType::BYTE:
      this->Internals->Image->AllocateScalars(VTK_UNSIGNED_CHAR, channelCount);
      break;
    case ChannelType::SHORT:
      this->Internals->Image->AllocateScalars(VTK_UNSIGNED_SHORT, channelCount);
      break;
    case ChannelType::FLOAT:
      this->Internals->Image->AllocateScalars(VTK_FLOAT, channelCount);
      break;
  }
}

//----------------------------------------------------------------------------
image::image(const std::string& path)
  : Internals(new image::internals())
{
  detail::init::initialize();

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);
  if (!vtksys::SystemTools::FileExists(fullPath))
  {
    throw read_exception("Cannot open file " + path);
  }

  auto reader = vtkSmartPointer<vtkImageReader2>::Take(
    vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));

  if (reader)
  {
    reader->SetFileName(fullPath.c_str());
    reader->Update();
    this->Internals->Image = reader->GetOutput();
  }

  if (!this->Internals->Image)
  {
    throw read_exception("Cannot read image " + path);
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
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
  this->Internals->Image->DeepCopy(img.Internals->Image);
}

//----------------------------------------------------------------------------
image& image::operator=(const image& img) noexcept
{
  this->Internals->Image = vtkSmartPointer<vtkImageData>::New();
  this->Internals->Image->DeepCopy(img.Internals->Image);
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
  int dims[3];
  this->Internals->Image->GetDimensions(dims);
  return dims[0];
}

//----------------------------------------------------------------------------
unsigned int image::getHeight() const
{
  int dims[3];
  this->Internals->Image->GetDimensions(dims);
  return dims[1];
}

#ifndef F3D_NO_DEPRECATED
//----------------------------------------------------------------------------
image& image::setResolution(unsigned int width, unsigned int height)
{
  this->Internals->Image->SetDimensions(width, height, 1);
  this->Internals->Image->AllocateScalars(VTK_UNSIGNED_CHAR, this->getChannelCount());
  return *this;
}
#endif

//----------------------------------------------------------------------------
unsigned int image::getChannelCount() const
{
  return this->Internals->Image->GetNumberOfScalarComponents();
}

#ifndef F3D_NO_DEPRECATED
//----------------------------------------------------------------------------
image& image::setChannelCount(unsigned int dim)
{
  this->Internals->Image->AllocateScalars(VTK_UNSIGNED_CHAR, dim);
  return *this;
}
#endif

//----------------------------------------------------------------------------
image::ChannelType image::getChannelType() const
{
  switch (this->Internals->Image->GetScalarType())
  {
    case VTK_UNSIGNED_CHAR:
      return ChannelType::BYTE;
    case VTK_UNSIGNED_SHORT:
      return ChannelType::SHORT;
    case VTK_FLOAT:
      return ChannelType::FLOAT;
  }
  throw read_exception("Unknown channel type");
}

//----------------------------------------------------------------------------
image& image::setData(unsigned char* buffer)
{
  int scalarSize = this->Internals->Image->GetScalarSize();
  int totalSize = this->getWidth() * this->getHeight() * this->getChannelCount() * scalarSize;
  unsigned char* internalBuffer =
    reinterpret_cast<unsigned char*>(this->Internals->Image->GetScalarPointer());
  std::copy(buffer, buffer + totalSize, internalBuffer);
  return *this;
}

//----------------------------------------------------------------------------
unsigned char* image::getData() const
{
  return reinterpret_cast<unsigned char*>(this->Internals->Image->GetScalarPointer());
}

//----------------------------------------------------------------------------
bool image::compare(const image& reference, double threshold, image& diff, double& error) const
{
  vtkNew<vtkImageDifference> imDiff;
  // handle threshold outside of vtkImageDifference:
  // https://gitlab.kitware.com/vtk/vtk/-/issues/18152
  imDiff->SetThreshold(0);
  imDiff->SetInputData(this->Internals->Image);
  imDiff->SetImageData(reference.Internals->Image);
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
    imDiff->Update();
    diff.Internals->Image = imDiff->GetOutput();
    return false;
  }
}

//----------------------------------------------------------------------------
bool image::operator==(const image& reference) const
{
  image diff;
  double error;
  return this->compare(reference, 0, diff, error);
}

//----------------------------------------------------------------------------
bool image::operator!=(const image& reference) const
{
  return !this->operator==(reference);
}

//----------------------------------------------------------------------------
void image::save(const std::string& path, SaveFormat format) const
{
  vtkSmartPointer<vtkImageWriter> writer;

  switch (format)
  {
    case SaveFormat::PNG:
      writer = vtkSmartPointer<vtkPNGWriter>::New();
      break;
    case SaveFormat::JPG:
      writer = vtkSmartPointer<vtkJPEGWriter>::New();
      break;
    case SaveFormat::TIF:
      writer = vtkSmartPointer<vtkTIFFWriter>::New();
      break;
    case SaveFormat::BMP:
      writer = vtkSmartPointer<vtkBMPWriter>::New();
      break;
    default:
      throw write_exception("Unknown format");
  }

  writer->SetFileName(path.c_str());
  writer->SetInputData(this->Internals->Image);
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
