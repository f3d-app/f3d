#include "vtkF3DPolyDataMapper.h"

#include <vtkCellData.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

int TestF3DPolyDataMapperColorSpace(int, char*[])
{
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0.0, 0.0, 0.0);
  vtkNew<vtkPolyData> data;
  data->SetPoints(points);

  vtkNew<vtkUnsignedCharArray> colors;
  colors->SetNumberOfComponents(4);
  const unsigned char rgba[4] = { 128, 64, 192, 127 };
  colors->InsertNextTypedTuple(rgba);
  data->GetPointData()->SetScalars(colors);
  data->GetCellData()->SetScalars(colors);

  vtkNew<vtkF3DPolyDataMapper> mapper;
  mapper->SetInputData(data);
  mapper->SetColorModeToDirectScalars();
  for (bool cells : { false, true })
  {
    mapper->SetScalarMode(cells ? VTK_SCALAR_MODE_USE_CELL_DATA : VTK_SCALAR_MODE_USE_POINT_DATA);
    for (bool linear : { false, true, false, true })
    {
      mapper->SetUseLinearColorSpace(linear);
      for (int repeat = 0; repeat < 2; ++repeat)
      {
        int cellFlag = -1;
        vtkUnsignedCharArray* mapped = mapper->MapScalars(data, 1.0, cellFlag);
        if (!mapped || mapped != mapper->GetColorMapColors() || cellFlag != (cells ? 1 : 0))
        {
          std::cerr << "Missing mapped scalar colors\n";
          return EXIT_FAILURE;
        }
        for (int component = 0; component < 4; ++component)
        {
          double expected = linear && component < 3
            ? std::round(std::pow(rgba[component] / 255.0, 2.2) * 255.0)
            : rgba[component];
          if (mapped->GetComponent(0, component) != expected ||
            colors->GetComponent(0, component) != rgba[component])
          {
            std::cerr << "Incorrect scalar color conversion or modified input\n";
            return EXIT_FAILURE;
          }
        }
      }
    }
  }

  colors->SetComponent(0, 0, 255);
  colors->Modified();
  if (mapper->MapScalars(1.0)->GetComponent(0, 0) != 255)
  {
    std::cerr << "Stale scalar colors after input update\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkLookupTable> lut;
  lut->SetNumberOfTableValues(2);
  lut->Build();
  lut->SetTableValue(0, 0.5, 0.25, 0.75, 0.5);
  lut->SetTableValue(1, 0.5, 0.25, 0.75, 0.5);
  mapper->SetLookupTable(lut);
  mapper->SetScalarModeToUsePointData();
  mapper->SetColorModeToMapScalars();
  mapper->InterpolateScalarsBeforeMappingOn();
  mapper->SetUseLinearColorSpace(false);
  mapper->MapScalars(1.0);
  vtkNew<vtkImageData> original;
  original->DeepCopy(mapper->GetColorTextureMap());

  for (bool linear : { true, false, true })
  {
    mapper->SetUseLinearColorSpace(linear);
    for (int repeat = 0; repeat < 2; ++repeat)
    {
      mapper->MapScalars(1.0);
      vtkImageData* texture = mapper->GetColorTextureMap();
      if (!texture || texture->GetScalarType() != (linear ? VTK_FLOAT : VTK_UNSIGNED_CHAR))
      {
        std::cerr << "Incorrect lookup texture format\n";
        return EXIT_FAILURE;
      }
      vtkDataArray* source = original->GetPointData()->GetScalars();
      vtkDataArray* mapped = texture->GetPointData()->GetScalars();
      for (vtkIdType i = 0; i < source->GetNumberOfTuples(); ++i)
      {
        for (int component = 0; component < 4; ++component)
        {
          double expected = source->GetComponent(i, component);
          if (linear)
          {
            expected /= 255.0;
            expected = component < 3 ? std::pow(expected, 2.2) : expected;
          }
          if (std::abs(mapped->GetComponent(i, component) - expected) > 1e-6)
          {
            std::cerr << "Incorrect lookup texture conversion\n";
            return EXIT_FAILURE;
          }
        }
      }
    }
  }

  lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
  mapper->MapScalars(1.0);
  if (mapper->GetColorTextureMap()->GetPointData()->GetScalars()->GetComponent(1, 0) != 1.0)
  {
    std::cerr << "Stale lookup texture after table update\n";
    return EXIT_FAILURE;
  }

  mapper->ScalarVisibilityOff();
  mapper->MapScalars(1.0);
  if (mapper->GetColorMapColors() || mapper->GetColorTextureMap())
  {
    std::cerr << "Scalar colors remain after disabling coloring\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
