#include "vtkF3DImageImporter.h"

#include <vtkCommand.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Collection.h>
#include <vtkImageReader2Factory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkResourceStream.h>
#include <vtkTexture.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DImageImporter);

//----------------------------------------------------------------------------
void vtkF3DImageImporter::ImportActors(vtkRenderer* renderer)
{
  const char* fileName = this->GetFileName();

  if (this->ImageHint.empty())
  {
    vtkErrorMacro("ImageHint not set");
    this->SetFailureStatus();
    return;
  }

  vtkSmartPointer<vtkImageReader2> reader;
  reader.TakeReference(
    vtkImageReader2Factory::CreateImageReader2FromExtension(this->ImageHint.c_str()));
  if (!reader)
  {
    vtkErrorMacro("Could not find a suitable image reader");
    this->SetFailureStatus();
    return;
  }

// needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12518
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251210)
  if (this->GetStream())
  {
    reader->SetStream(this->GetStream());
  }
  else
#endif
    if (fileName)
  {
    reader->SetFileName(fileName);
  }

  reader->UpdateInformation();

  int extent[6];
  reader->GetDataExtent(extent);

  assert(extent[4] == extent[5]); // only support single slice

  int w = extent[1] - extent[0] + 1;
  int h = extent[3] - extent[2] + 1;

  vtkNew<vtkTexture> texture;
  texture->SetInputConnection(reader->GetOutputPort());
  texture->SetColorModeToDirectScalars();

  vtkNew<vtkPolyData> polydata;

  // fill the polydata with a single quad
  polydata->Allocate(1);
  vtkIdType pts[4] = { 0, 1, 2, 3 };
  polydata->InsertNextCell(VTK_QUAD, 4, pts);

  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(4);
  points->SetPoint(0, 0.0, 0.0, 0.0);
  points->SetPoint(1, static_cast<double>(w), 0.0, 0.0);
  points->SetPoint(2, static_cast<double>(w), static_cast<double>(h), 0.0);
  points->SetPoint(3, 0.0, static_cast<double>(h), 0.0);

  polydata->SetPoints(points);
  vtkNew<vtkFloatArray> tcoords;
  tcoords->SetNumberOfComponents(2);
  tcoords->SetNumberOfTuples(4);
  tcoords->SetTuple2(0, 0.0, 0.0);
  tcoords->SetTuple2(1, 1.0, 0.0);
  tcoords->SetTuple2(2, 1.0, 1.0);
  tcoords->SetTuple2(3, 0.0, 1.0);
  polydata->GetPointData()->SetTCoords(tcoords);

  vtkNew<vtkActor> actor;
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polydata);
  actor->SetMapper(mapper);

  texture->UseSRGBColorSpaceOn();
  actor->GetProperty()->SetInterpolationToPBR();
  actor->GetProperty()->SetBaseColorTexture(texture);
  actor->GetProperty()->SetBaseIOR(1.0);
  renderer->AddActor(actor);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->ActorCollection->AddItem(actor);
#endif
}

//------------------------------------------------------------------------------
bool vtkF3DImageImporter::CanReadFile(vtkResourceStream* stream)
{
  std::string unused;
  return vtkF3DImageImporter::CanReadFile(stream, unused);
}

//------------------------------------------------------------------------------
bool vtkF3DImageImporter::CanReadFile(vtkResourceStream* stream, std::string& hint)
{
  if (!stream)
  {
    return false;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260128)
  vtkNew<vtkImageReader2Collection> collection;
  vtkImageReader2Factory::GetRegisteredReaders(collection);
  collection->InitTraversal();
  while (vtkImageReader2* probe = collection->GetNextItem())
  {
    // We assume that there is no two image readers that return
    // a positive value on the CanReadFile call
    if (probe->CanReadFile(stream) > 0)
    {
      // Space separated list of supported extensions
      std::string extensions = probe->GetFileExtensions();

      // Extract the first as a hint
      hint = extensions.substr(1, extensions.find_first_of(' ') - 1);
      return true;
    }
  }

  return false;
#else
  // Unreachable
  return true;
#endif
}
