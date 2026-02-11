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
#include <vtkRenderer.h>
#include <vtkTexture.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DImageImporter);

//----------------------------------------------------------------------------
void vtkF3DImageImporter::ImportActors(vtkRenderer* renderer)
{
  const char* fileName = this->GetFileName();

  vtkSmartPointer<vtkImageReader2> reader;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  if (this->GetStream())
  {
    vtkNew<vtkImageReader2Collection> collection;
    vtkImageReader2Factory::GetRegisteredReaders(collection);
    collection->InitTraversal();
    while (vtkImageReader2* probe = collection->GetNextItem())
    {
      if (probe->CanReadFile(this->GetStream()))
      {
        reader.TakeReference(probe->NewInstance());
        reader->SetStream(this->GetStream());
        break;
      }
    }
  }
  else
#endif
    if (fileName)
  {
    reader.TakeReference(vtkImageReader2Factory::CreateImageReader2(fileName));
    reader->SetFileName(fileName);
  }

  if (!reader)
  {
    vtkErrorMacro("Could not find a suitable image reader");
    return;
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
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(static_cast<double>(w), 0.0, 0.0);
  points->InsertNextPoint(static_cast<double>(w), static_cast<double>(h), 0.0);
  points->InsertNextPoint(0.0, static_cast<double>(h), 0.0);

  polydata->SetPoints(points);
  vtkNew<vtkFloatArray> tcoords;
  tcoords->SetNumberOfComponents(2);
  tcoords->InsertNextTuple2(0.0, 0.0);
  tcoords->InsertNextTuple2(1.0, 0.0);
  tcoords->InsertNextTuple2(1.0, 1.0);
  tcoords->InsertNextTuple2(0.0, 1.0);
  polydata->GetPointData()->SetTCoords(tcoords);

  vtkNew<vtkActor> actor;
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polydata);
  actor->SetMapper(mapper);

  actor->SetTexture(texture);
  renderer->AddActor(actor);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->ActorCollection->AddItem(actor);
#endif
}
