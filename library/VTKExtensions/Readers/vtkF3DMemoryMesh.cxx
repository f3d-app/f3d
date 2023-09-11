#include "vtkF3DMemoryMesh.h"

#include "vtkDataArrayRange.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

#include <execution>
#include <numeric>

vtkStandardNewMacro(vtkF3DMemoryMesh);

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
  vtkNew<vtkFloatArray> arr;
  arr->SetNumberOfComponents(3);
  arr->SetNumberOfTuples(positions.size() / 3);

  std::copy(std::execution::par, positions.cbegin(), positions.cend(), arr->GetPointer(0));

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(arr);

  this->Mesh->SetPoints(points);
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
  std::inclusive_scan(std::execution::par, faceSizes.cbegin(), faceSizes.cend(),
    vtk::DataArrayValueRange(offsets).begin() + 1,
    [](unsigned int a, unsigned int b) { return static_cast<vtkIdType>(a + b); });

  // fill connectivity
  std::transform(std::execution::par, faceIndices.cbegin(), faceIndices.cend(),
    vtk::DataArrayValueRange(connectivity).begin(),
    [](unsigned int i) { return static_cast<vtkIdType>(i); });

  vtkNew<vtkCellArray> polys;
  polys->SetData(offsets, connectivity);

  this->Mesh->SetPolys(polys);
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  output->ShallowCopy(this->Mesh);

  return 1;
}
