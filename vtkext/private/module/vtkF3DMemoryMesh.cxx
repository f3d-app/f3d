#include "vtkF3DMemoryMesh.h"

#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSMPTools.h"
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

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
  this->SetNumberOfInputPorts(0);
}

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::~vtkF3DMemoryMesh() = default;

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetPoints(const std::vector<float>& positions, const double timeStamp)
{
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(ConvertToFloatArray<3>(positions));

  this->Meshes[timeStamp]->SetPoints(points);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetNormals(const std::vector<float>& normals, const double timeStamp)
{
  this->Meshes[timeStamp]->GetPointData()->SetNormals(ConvertToFloatArray<3>(normals));
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTCoords(const std::vector<float>& tcoords, const double timeStamp)
{
  this->Meshes[timeStamp]->GetPointData()->SetTCoords(ConvertToFloatArray<2>(tcoords));
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetFaces(const std::vector<unsigned int>& faceSizes,
  const std::vector<unsigned int>& faceIndices, const double timeStamp)
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

  this->Meshes[timeStamp]->SetPolys(polys);
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // timeRange = {map_min, map_max}, map_min always <= map_max
  const auto timeRange = std::array<double, 2>{ Meshes.begin()->first, Meshes.rbegin()->first };

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange.data(), 2);

  return 1;
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::GetData(outInfo);

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) && !this->Meshes.empty())
  {
    const double requestedTimeValue =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    auto iter = Meshes.upper_bound(requestedTimeValue);
    if (iter != Meshes.begin())
    {
      iter = std::prev(iter);
    }
    output->ShallowCopy(iter->second);
  }
  else
  {
    output->ShallowCopy(this->Meshes[0.0]);
  }

  return 1;
}
