#include "vtkF3DMemoryMesh.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSMPTools.h"
#include "vtkStreamingDemandDrivenPipeline.h"

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
void vtkF3DMemoryMesh::SetPoints(const std::vector<float>& positions)
{
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(ConvertToFloatArray<3>(positions));
  this->StaticMesh->SetPoints(points);
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetNormals(const std::vector<float>& normals)
{
  if (!normals.empty())
  {
    this->StaticMesh->GetPointData()->SetNormals(ConvertToFloatArray<3>(normals));
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTCoords(const std::vector<float>& tcoords)
{
  if (!tcoords.empty())
  {
    this->StaticMesh->GetPointData()->SetTCoords(ConvertToFloatArray<2>(tcoords));
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetFaces(
  const std::vector<unsigned int>& faceSizes, const std::vector<unsigned int>& faceIndices)
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
  this->StaticMesh->SetPolys(polys);
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetAnimatedMesh(double startTime, double endTime, MeshCallback callback)
{
  this->AnimatedCallback = std::move(callback);
  this->TimeRange[0] = startTime;
  this->TimeRange[1] = endTime;
  this->IsAnimated = true;
  this->Modified();
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (this->IsAnimated)
  {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), this->TimeRange, 2);
  }

  return 1;
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::GetData(outInfo);

  if (this->IsAnimated && this->AnimatedCallback)
  {
    double requestedTime = this->TimeRange[0];
    if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
      requestedTime = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    }
    this->AnimatedCallback(requestedTime, this);
  }

  output->ShallowCopy(this->StaticMesh);

  return 1;
}
