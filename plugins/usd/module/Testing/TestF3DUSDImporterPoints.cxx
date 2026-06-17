#include "vtkF3DUSDImporter.h"

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

#include <cmath>
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

bool RgbaEquals(vtkDataArray* colors, vtkIdType index, float r, float g, float b, float a)
{
  double rgba[4];
  colors->GetTuple(index, rgba);
  const double tol = 1e-5;
  return std::abs(rgba[0] - r) < tol && std::abs(rgba[1] - g) < tol &&
    std::abs(rgba[2] - b) < tol && std::abs(rgba[3] - a) < tol;
}

vtkPolyData* GetPointsPolyData(vtkF3DUSDImporter* importer)
{
  vtkRenderer* renderer = importer->GetRenderer();
  if (!renderer)
  {
    return nullptr;
  }

  vtkActorCollection* actors = renderer->GetActors();
  actors->InitTraversal();

  while (vtkActor* actor = actors->GetNextActor())
  {
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    if (!mapper)
    {
      continue;
    }

    vtkPolyData* polydata = vtkPolyData::SafeDownCast(mapper->GetInput());
    if (polydata && IsPointsPolyData(polydata))
    {
      return polydata;
    }
  }

  return nullptr;
}
}

int TestF3DUSDImporterPoints(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/usd_points_rgb.usda";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->Update();

  vtkPolyData* polydata = GetPointsPolyData(importer);
  if (!polydata)
  {
    std::cerr << "Missing polydata for USD Points primitive\n";
    return EXIT_FAILURE;
  }

  if (polydata->GetNumberOfPoints() != 3)
  {
    std::cerr << "Unexpected point count for USD Points primitive\n";
    return EXIT_FAILURE;
  }

  vtkDataArray* colors = polydata->GetPointData()->GetScalars();
  if (!colors || std::strcmp(colors->GetName(), "RGBA") != 0 ||
    colors->GetNumberOfComponents() != 4 || colors->GetDataType() != VTK_FLOAT)
  {
    std::cerr << "Missing RGBA scalars on USD Points primitive\n";
    return EXIT_FAILURE;
  }

  if (!RgbaEquals(colors, 0, 1.f, 0.f, 0.f, 1.f) || !RgbaEquals(colors, 1, 0.f, 1.f, 0.f, 0.5f) ||
    !RgbaEquals(colors, 2, 0.f, 0.f, 1.f, 0.25f))
  {
    std::cerr << "Unexpected displayColor/displayOpacity values on USD Points primitive\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
