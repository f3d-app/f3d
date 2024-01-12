#include "vtkF3DFaceVaryingPolyData.h"

#include "vtkInformationVector.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

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

  vtkDataArray* originalNormals = input->GetPointData()->GetNormals();
  vtkDataArray* originalTCoords = input->GetPointData()->GetTCoords();

  bool hasNormals = originalNormals && originalNormals->GetNumberOfTuples() > 0;
  bool hasTCoords = originalTCoords && originalTCoords->GetNumberOfTuples() > 0;

  if (!(hasNormals && this->NormalsFaceVarying) && !(hasTCoords && this->TCoordsFaceVarying))
  {
    output->ShallowCopy(input);
    return 1;
  }

  vtkPoints* originalPoints = input->GetPoints();
  vtkCellArray* originalFaces = input->GetPolys();

  vtkNew<vtkPoints> newPoints;
  vtkNew<vtkCellArray> newFaces;
  vtkNew<vtkPointData> newPointData;
  vtkNew<vtkFloatArray> newNormals;
  vtkNew<vtkFloatArray> newTCoords;

  vtkIdType nbCells = originalFaces->GetNumberOfCells();
  vtkIdType nbConnectivity = originalFaces->GetNumberOfConnectivityIds();

  newPoints->SetNumberOfPoints(nbConnectivity);

  if (hasNormals)
  {
    if (this->NormalsFaceVarying)
    {
      output->GetPointData()->SetNormals(originalNormals);
    }
    else
    {
      newNormals->SetNumberOfComponents(3);
      newNormals->SetNumberOfTuples(nbConnectivity);

      output->GetPointData()->SetNormals(newNormals);
    }
  }

  if (hasTCoords)
  {
    if (this->TCoordsFaceVarying)
    {
      output->GetPointData()->SetTCoords(originalTCoords);
    }
    else
    {
      newTCoords->SetNumberOfComponents(2);
      newTCoords->SetNumberOfTuples(nbConnectivity);

      output->GetPointData()->SetTCoords(newTCoords);
    }
  }

  std::vector<vtkIdType> cell;
  cell.resize(originalFaces->GetMaxCellSize());

  vtkIdType currentVertexIndex = 0;

  for (vtkIdType i = 0; i < nbCells; i++)
  {
    vtkIdType cellSize;
    const vtkIdType* cellPoints;
    originalFaces->GetCellAtId(i, cellSize, cellPoints);

    for (vtkIdType j = 0; j < cellSize; j++)
    {
      cell[j] = currentVertexIndex + j;
      newPoints->SetPoint(currentVertexIndex + j, originalPoints->GetPoint(cellPoints[j]));

      if (hasNormals && !this->NormalsFaceVarying)
      {
        newNormals->SetTuple(currentVertexIndex + j, originalNormals->GetTuple(cellPoints[j]));
      }

      if (hasTCoords && !this->TCoordsFaceVarying)
      {
        newTCoords->SetTuple(currentVertexIndex + j, originalTCoords->GetTuple(cellPoints[j]));
      }
    }

    newFaces->InsertNextCell(cellSize, cell.data());

    currentVertexIndex += cellSize;
  }

  output->SetPoints(newPoints);
  output->SetPolys(newFaces);

  return 1;
}
