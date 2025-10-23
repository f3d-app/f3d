#include "vtkF3DMemoryMesh.h"

#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSMPTools.h"
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <algorithm>
#include <cmath>

vtkStandardNewMacro(vtkF3DMemoryMesh);

namespace
{
template<vtkIdType NbComponents>
vtkSmartPointer<vtkFloatArray> ConvertToFloatArray(const std::vector<float>& positions)
{
  vtkIdType nbVertices = static_cast<vtkIdType>(positions.size() / NbComponents);

  vtkNew<vtkFloatArray> arr;
  arr->SetNumberOfComponents(NbComponents);
  arr->SetNumberOfTuples(nbVertices);

  vtkSMPTools::For(0, nbVertices,
    [&](vtkIdType begin, vtkIdType end)
    {
      for (vtkIdType i = begin; i < end; i++)
      {
        arr->SetTypedTuple(i, positions.data() + NbComponents * i);
      }
    });

  return arr;
}
}

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::vtkF3DMemoryMesh()
{
  Meshes = std::vector<vtkNew<vtkPolyData>>(1);
  this->SetNumberOfInputPorts(0);
  this->timeRange = { 0.0, 0.0 };
  this->timeStep = 1.0;
}

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::~vtkF3DMemoryMesh() = default;

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetPoints(const std::vector<float>& positions, size_t timeIdx)
{
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(ConvertToFloatArray<3>(positions));

  this->Meshes[timeIdx]->SetPoints(points);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetNormals(const std::vector<float>& normals, size_t timeIdx)
{
  this->Meshes[timeIdx]->GetPointData()->SetNormals(ConvertToFloatArray<3>(normals));
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTCoords(const std::vector<float>& tcoords, size_t timeIdx)
{
  this->Meshes[timeIdx]->GetPointData()->SetTCoords(ConvertToFloatArray<2>(tcoords));
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetFaces(const std::vector<unsigned int>& faceSizes,
  const std::vector<unsigned int>& faceIndices, size_t timeIdx)
{
  vtkNew<vtkIdTypeArray> offsets;
  vtkNew<vtkIdTypeArray> connectivity;
  offsets->SetNumberOfTuples(faceSizes.size() + 1);
  connectivity->SetNumberOfTuples(faceIndices.size());

  // fill offsets
  offsets->SetTypedComponent(0, 0, 0);
  for (size_t i = 0; i < faceSizes.size(); i++)
  {
    offsets->SetTypedComponent(
      i + 1, 0, offsets->GetTypedComponent(i, 0) + static_cast<vtkIdType>(faceSizes[i]));
  }

  // fill connectivity
  vtkSMPTools::For(0, faceIndices.size(),
    [&](vtkIdType begin, vtkIdType end)
    {
      for (vtkIdType i = begin; i < end; i++)
      {
        connectivity->SetTypedComponent(i, 0, static_cast<vtkIdType>(faceIndices[i]));
      }
    });

  vtkNew<vtkCellArray> polys;
  polys->SetData(offsets, connectivity);

  this->Meshes[timeIdx]->SetPolys(polys);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTimeRange(std::pair<double, double> newTimeRange)
{
  timeRange = newTimeRange;
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTimeStep(double newTimeStep)
{
  timeStep = newTimeStep;
}

//------------------------------------------------------------------------------
size_t vtkF3DMemoryMesh::GetTemporalStateCount()
{
  return Meshes.size();
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::ReserveTemporalEntries(size_t newSize)
{
  Meshes.resize(newSize);
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (timeRange.first < timeRange.second)
  {
    std::array<double, 2> arr = { timeRange.first, timeRange.second };
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), arr.data(), 2);
  }

  return 1;
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::GetData(outInfo);

  double requestedTimeValue = 0.0;
  size_t timeIdx = 0;

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) && this->timeStep > 0.0 &&
    !this->Meshes.empty())
  {
    requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    const double clampedTime =
      std::clamp(requestedTimeValue, this->timeRange.first, this->timeRange.second);
    const double relative = (clampedTime - this->timeRange.first) / this->timeStep;
    const double boundedRelative =
      std::clamp(relative, 0.0, static_cast<double>(std::max<size_t>(this->Meshes.size(), 1) - 1));
    timeIdx = static_cast<size_t>(std::llround(boundedRelative));
  }

  output->ShallowCopy(this->Meshes[timeIdx]);

  return 1;
}
