#include "vtkF3DMemoryMesh.h"

#include "vtkDataArrayRange.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSMPTools.h"

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
  vtkIdType nbVertices = static_cast<vtkIdType>(positions.size() / 3);

  vtkNew<vtkFloatArray> arr;
  arr->SetNumberOfComponents(3);
  arr->SetNumberOfTuples(nbVertices);

  vtkSMPTools::For(0, nbVertices,
    [&](vtkIdType begin, vtkIdType end)
    {
      for (vtkIdType i = begin; i < end; i++)
      {
        arr->SetTypedTuple(i, positions.data() + 3 * i);
      }
    });

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
  std::inclusive_scan(faceSizes.cbegin(), faceSizes.cend(),
    vtk::DataArrayValueRange(offsets).begin() + 1,
    [](unsigned int a, unsigned int b) { return static_cast<vtkIdType>(a + b); });

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
