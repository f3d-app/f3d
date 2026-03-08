#include "vtkF3DPDALReader.h"

#include "vtkDoubleArray.h"
#include "vtkTypeUInt16Array.h"
#include "vtkPointData.h"

#include <iostream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DPDALReader);

//----------------------------------------------------------------------------
int vtkF3DPDALReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  if (this->Superclass::RequestData(nullptr, nullptr, outputVector) == 0)
  {
    return 0;
  }

  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  vtkPointData* pointData = output->GetPointData();
  vtkTypeUInt16Array* colors = vtkTypeUInt16Array::SafeDownCast(pointData->GetArray("Color"));
  if (colors)
  {
    // Normalize colors in unsigned char array for rendering
    vtkNew<vtkDoubleArray> normalizedColors;
    normalizedColors->SetNumberOfComponents(3);
    normalizedColors->SetName("NormalizedColor");

    double divider = std::numeric_limits<std::uint8_t>::max(); 
    double colorRanges[3][2];
    for (int j = 0; j < 3; j++)
    {
      colors->GetRange(colorRanges[j], j);
      if (colorRanges[j][1] > std::numeric_limits<std::uint8_t>::max())
      {
        divider = std::numeric_limits<std::uint16_t>::max();
      }
    }

    for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); i++)
    {
      vtkTypeUInt16 color[3];
      double divided[3];
      colors->GetTypedTuple(i, color);

      for (int j = 0; j < 3; j++)
      {
        divided[j] = color[j] / divider;
      }
      normalizedColors->InsertNextTuple(divided);
    }
    pointData->AddArray(normalizedColors);
  }

  return 1;
}
