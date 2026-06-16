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

bool ColorEquals(vtkDataArray* colors, vtkIdType index, float r, float g, float b)
{
  double rgb[3];
  colors->GetTuple(index, rgb);
  const double tol = 1e-5;
  return std::abs(rgb[0] - r) < tol && std::abs(rgb[1] - g) < tol && std::abs(rgb[2] - b) < tol;
}

bool RgbaEquals(vtkDataArray* colors, vtkIdType index, float r, float g, float b, float a)
{
  double rgba[4];
  colors->GetTuple(index, rgba);
  const double tol = 1e-5;
  return std::abs(rgba[0] - r) < tol && std::abs(rgba[1] - g) < tol && std::abs(rgba[2] - b) < tol &&
    std::abs(rgba[3] - a) < tol;
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
  const std::string dataDir = std::string(argv[1]) + "data/";

  {
    vtkNew<vtkF3DUSDImporter> importer;
    importer->SetFileName((dataDir + "skeleton_skin_basic.usda").c_str());
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
    if (!colors || std::strcmp(colors->GetName(), "RGB") != 0 ||
      colors->GetNumberOfComponents() != 3 || colors->GetDataType() != VTK_FLOAT)
    {
      std::cerr << "Missing RGB scalars on USD Points primitive\n";
      return EXIT_FAILURE;
    }

    if (!ColorEquals(colors, 0, 1.f, 0.f, 0.f) || !ColorEquals(colors, 1, 0.f, 1.f, 0.f) ||
      !ColorEquals(colors, 2, 0.f, 0.f, 1.f))
    {
      std::cerr << "Unexpected displayColor values on USD Points primitive\n";
      return EXIT_FAILURE;
    }

    if (!polydata->GetPointData()->GetArray("JOINTS_0") ||
      !polydata->GetPointData()->GetArray("WEIGHTS_0"))
    {
      std::cerr << "Missing skinning attributes on USD Points primitive\n";
      return EXIT_FAILURE;
    }
  }

  {
    vtkNew<vtkF3DUSDImporter> importer;
    importer->SetFileName((dataDir + "usd_points_rgba.usda").c_str());
    importer->Update();

    vtkPolyData* polydata = GetPointsPolyData(importer);
    if (!polydata)
    {
      std::cerr << "Missing polydata for USD Points primitive with opacity\n";
      return EXIT_FAILURE;
    }

    if (polydata->GetNumberOfPoints() != 3)
    {
      std::cerr << "Unexpected point count for USD Points primitive with opacity\n";
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
  }

  return EXIT_SUCCESS;
}
