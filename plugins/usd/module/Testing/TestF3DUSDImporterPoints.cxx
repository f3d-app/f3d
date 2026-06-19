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
  return std::abs(rgba[0] - r) < tol && std::abs(rgba[1] - g) < tol &&
    std::abs(rgba[2] - b) < tol && std::abs(rgba[3] - a) < tol;
}

vtkPolyData* GetPointsPolyData(vtkF3DUSDImporter* importer, vtkIdType expectedPointCount)
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
    if (polydata && IsPointsPolyData(polydata) &&
      polydata->GetNumberOfPoints() == expectedPointCount)
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

  vtkPolyData* rgbaPolydata = GetPointsPolyData(importer, 2);
  if (!rgbaPolydata)
  {
    std::cerr << "Missing RGBA polydata for USD Points primitive\n";
    return EXIT_FAILURE;
  }

  vtkDataArray* rgbaColors = rgbaPolydata->GetPointData()->GetScalars();
  if (!rgbaColors || std::strcmp(rgbaColors->GetName(), "RGBA") != 0 ||
    rgbaColors->GetNumberOfComponents() != 4 || rgbaColors->GetDataType() != VTK_FLOAT)
  {
    std::cerr << "Missing RGBA scalars on USD Points primitive with opacity\n";
    return EXIT_FAILURE;
  }

  if (!RgbaEquals(rgbaColors, 0, 1.f, 0.f, 0.f, 1.f) ||
    !RgbaEquals(rgbaColors, 1, 0.f, 1.f, 0.f, 0.5f))
  {
    std::cerr << "Unexpected displayColor/displayOpacity values on USD Points primitive\n";
    return EXIT_FAILURE;
  }

  vtkPolyData* rgbPolydata = GetPointsPolyData(importer, 1);
  if (!rgbPolydata)
  {
    std::cerr << "Missing RGB polydata for USD Points primitive without opacity\n";
    return EXIT_FAILURE;
  }

  vtkDataArray* rgbColors = rgbPolydata->GetPointData()->GetScalars();
  if (!rgbColors || std::strcmp(rgbColors->GetName(), "RGB") != 0 ||
    rgbColors->GetNumberOfComponents() != 3 || rgbColors->GetDataType() != VTK_FLOAT)
  {
    std::cerr << "Missing RGB scalars on USD Points primitive without opacity\n";
    return EXIT_FAILURE;
  }

  if (!ColorEquals(rgbColors, 0, 0.f, 0.f, 1.f))
  {
    std::cerr << "Unexpected displayColor values on USD Points primitive without opacity\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
