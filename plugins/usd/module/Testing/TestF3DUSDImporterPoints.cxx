#include "vtkF3DUSDImporter.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

#include <cstring>
#include <iostream>
#include <string>

namespace
{
bool IsPointsPolyData(vtkPolyData* polydata)
{
  return polydata->GetNumberOfPoints() > 0 && polydata->GetNumberOfVerts() > 0 &&
    polydata->GetNumberOfPolys() == 0 && polydata->GetNumberOfLines() == 0;
}
}

int TestF3DUSDImporterPoints(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/skeleton_skin_basic.usda";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->Update();

  vtkRenderer* renderer = importer->GetRenderer();
  if (!renderer)
  {
    std::cerr << "Missing renderer\n";
    return EXIT_FAILURE;
  }

  vtkActorCollection* actors = renderer->GetActors();
  actors->InitTraversal();

  bool foundPointsActor = false;
  while (vtkActor* actor = actors->GetNextActor())
  {
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    if (!mapper)
    {
      continue;
    }

    vtkPolyData* polydata = vtkPolyData::SafeDownCast(mapper->GetInput());
    if (!polydata || !IsPointsPolyData(polydata))
    {
      continue;
    }

    if (polydata->GetNumberOfPoints() != 3)
    {
      std::cerr << "Unexpected point count for USD Points primitive\n";
      return EXIT_FAILURE;
    }

    vtkDataArray* colors = polydata->GetPointData()->GetScalars();
    if (!colors || std::strcmp(colors->GetName(), "RGB") != 0 ||
      colors->GetNumberOfComponents() != 3 || colors->GetDataType() != VTK_FLOAT)
    {
      std::cerr << "Missing RGB scalars on USD Points primitive\n";
      return EXIT_FAILURE;
    }

    if (!polydata->GetPointData()->GetArray("JOINTS_0") ||
      !polydata->GetPointData()->GetArray("WEIGHTS_0"))
    {
      std::cerr << "Missing skinning attributes on USD Points primitive\n";
      return EXIT_FAILURE;
    }

    foundPointsActor = true;
    break;
  }

  if (!foundPointsActor)
  {
    std::cerr << "Missing polydata for USD Points primitive\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
