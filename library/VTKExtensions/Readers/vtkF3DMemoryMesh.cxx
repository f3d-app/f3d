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

  this->Mesh->SetPoints(points);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetNormals(const std::vector<float>& normals)
{
  this->Mesh->GetPointData()->SetNormals(ConvertToFloatArray<3>(normals));
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTCoords(const std::vector<float>& tcoords)
{
  this->Mesh->GetPointData()->SetTCoords(ConvertToFloatArray<2>(tcoords));
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
