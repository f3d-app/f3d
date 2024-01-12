#include "vtkF3DFaceVaryingPolyData.h"

#include "vtkInformationVector.h"

vtkStandardNewMacro(vtkF3DFaceVaryingPolyData);

//------------------------------------------------------------------------------
vtkF3DFaceVaryingPolyData::vtkF3DFaceVaryingPolyData() = default;

//------------------------------------------------------------------------------
vtkF3DFaceVaryingPolyData::~vtkF3DFaceVaryingPolyData() = default;

//------------------------------------------------------------------------------
int vtkF3DFaceVaryingPolyData::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]->GetInformationObject(0));
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  output->ShallowCopy(input);

  return 1;
}
