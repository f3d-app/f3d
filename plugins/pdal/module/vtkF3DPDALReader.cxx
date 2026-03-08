#include "vtkF3DPDALReader.h"

#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkSMPTools.h"
#include "vtkTypeUInt16Array.h"

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

    // Identify the divider to use, colors can be uint8 or uint16
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

    // Convert into [0,1] doubles
    vtkNew<vtkDoubleArray> normalizedColors;
    normalizedColors->SetNumberOfComponents(3);
    normalizedColors->SetName("NormalizedColor");
    normalizedColors->SetNumberOfTuples(colors->GetNumberOfTuples());

    vtkSMPTools::For(0, colors->GetNumberOfTuples(),
      [&](vtkIdType begin, vtkIdType end)
      {
        for (vtkIdType cc = begin; cc < end; ++cc)
        {
          vtkTypeUInt16 in[3];
          colors->GetTypedTuple(cc, in);

          double out[3];
          out[0] = in[0] / divider;
          out[1] = in[1] / divider;
          out[2] = in[2] / divider;
          normalizedColors->SetTypedTuple(cc, out);
        }
      });

    pointData->AddArray(normalizedColors);
  }

  return 1;
}
