#include "vtkF3DMemoryMesh.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkF3DMemoryMesh);

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::vtkF3DMemoryMesh()
{
  this->SetNumberOfInputPorts(0);
}

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::~vtkF3DMemoryMesh() = default;

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTimeRange(double startTime, double endTime)
{
  this->TimeRange[0] = startTime;
  this->TimeRange[1] = endTime;
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetUpdateFunction(std::function<void(double, vtkPolyData*)> updateFunction)
{
  this->UpdateFunction = std::move(updateFunction);
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (this->TimeRange[0] < this->TimeRange[1])
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), this->TimeRange, 2);
  }
  return 1;
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  assert(this->UpdateFunction);

  double time = 0.0;
  if (outputVector->GetInformationObject(0)->Has(
        vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
  {
    time = outputVector->GetInformationObject(0)->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
  }
  this->UpdateFunction(time, output);

  return 1;
}
