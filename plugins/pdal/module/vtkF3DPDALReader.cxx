#include "vtkF3DPDALReader.h"

#include "vtkPointData.h"
#include "vtkDoubleArray.h"

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
  vtkDataArray* colors = pointData->GetArray("Color");
  if (colors)
  {
    // Normalize colors in unsigned char array for rendering
    vtkNew<vtkDoubleArray> normalizedColors;
    normalizedColors->SetNumberOfComponents(3);
    normalizedColors->SetName("NormalizedColor");
    
    // TODO PROPER API 
    double colorRanges[3][2];
    for (int j = 0; j < 3; j++)
    {
      colors->GetRange(colorRanges[j], j);
      if (colorRanges[j][1] == 0)
      {
        colorRanges[j][1] = 1;
      }
    }

    for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); i++)
    {
      vtkTypeUInt64 color[3];
      double normalized[3];
      colors->GetUnsignedTuple(i, color);

      for (int j = 0; j < 3; j++)
      {
        normalized[j] = (color[j] - colorRanges[j][0]) / colorRanges[j][1];
      }
      normalizedColors->InsertNextTuple(normalized); 
    }
    pointData->AddArray(normalizedColors);
  }

  return 1;
}
