#include "vtkF3DMemoryMesh.h"

#include "vtkFloatArray.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkF3DMemoryMesh);

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::vtkF3DMemoryMesh()
{
  this->SetNumberOfInputPorts(0);
}

//------------------------------------------------------------------------------
vtkF3DMemoryMesh::~vtkF3DMemoryMesh() = default;

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetPoints(const std::vector<float>& positions)
{
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(positions.size() / 3);

  for (size_t i = 0; i < positions.size() / 3; i++)
  {
    points->SetPoint(i, &positions[3 * i]);
  }

  this->Mesh->SetPoints(points);
}

//------------------------------------------------------------------------------
void vtkF3DMemoryMesh::SetTriangles(const std::vector<unsigned int>& triangles)
{
  vtkNew<vtkCellArray> cells;
  cells->AllocateExact(triangles.size() / 3, triangles.size());

  for (size_t i = 0; i < triangles.size(); i += 3)
  {
    vtkIdType t[] = { static_cast<vtkIdType>(triangles[i]),
      static_cast<vtkIdType>(triangles[i + 1]), static_cast<vtkIdType>(triangles[i + 2]) };
    cells->InsertNextCell(3, t);
  }

  this->Mesh->SetPolys(cells);
}

//------------------------------------------------------------------------------
int vtkF3DMemoryMesh::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  output->ShallowCopy(this->Mesh);

  return 1;
}
