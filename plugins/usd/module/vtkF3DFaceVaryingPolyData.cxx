#include "vtkF3DFaceVaryingPolyData.h"

#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkF3DFaceVaryingPolyData);

vtkInformationKeyMacro(vtkF3DFaceVaryingPolyData, INTERPOLATION_TYPE, Integer);

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

  vtkPoints* originalPoints = input->GetPoints();
  vtkCellArray* originalFaces = input->GetPolys();

  // todo: early exit if all vertex

  vtkNew<vtkPoints> newPoints;
  vtkNew<vtkCellArray> newFaces;

  vtkIdType nbCells = originalFaces->GetNumberOfCells();
  vtkIdType nbConnectivity = originalFaces->GetNumberOfConnectivityIds();

  newPoints->SetNumberOfPoints(nbConnectivity);

  vtkPointData* originalPointData = input->GetPointData();
  vtkPointData* newPointData = output->GetPointData();

  vtkIdType nbArrays = originalPointData->GetNumberOfArrays();

  for (vtkIdType i = 0; i < nbArrays; i++)
  {
    vtkDataArray* originalArray = originalPointData->GetArray(i);

    vtkInformation* info = originalArray->GetInformation();
    int interpType = info->Get(vtkF3DFaceVaryingPolyData::INTERPOLATION_TYPE());

    if (interpType == 0) // vertex
    {
      vtkDataArray* newArray = vtkDataArray::CreateDataArray(originalArray->GetDataType());

      // TODO: Assumes float array, to generalize
      vtkFloatArray* newTypedArray = vtkFloatArray::SafeDownCast(newArray);
      if (newTypedArray)
      {
        newTypedArray->SetNumberOfComponents(originalArray->GetNumberOfComponents());
        newTypedArray->SetNumberOfTuples(nbConnectivity);
        newTypedArray->SetName(originalArray->GetName());

        newPointData->AddArray(newTypedArray);
      }
    }
    else
    {
      // if faceVarying, just copy
      newPointData->AddArray(originalArray);
    }
  }

  // set attributes
  for (int attribute : { vtkDataSetAttributes::NORMALS, vtkDataSetAttributes::TCOORDS })
  {
    vtkDataArray* array = originalPointData->GetAttribute(attribute);
    if (array)
    {
      newPointData->SetActiveAttribute(array->GetName(), attribute);
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

      for (vtkIdType i = 0; i < nbArrays; i++)
      {
        vtkDataArray* originalArray = originalPointData->GetArray(i);

        vtkInformation* info = originalArray->GetInformation();
        int interpType = info->Get(vtkF3DFaceVaryingPolyData::INTERPOLATION_TYPE());

        if (interpType == 0) // vertex
        {
          vtkDataArray* newArray = newPointData->GetArray(originalArray->GetName());

          if (newArray)
          {
            newArray->SetTuple(currentVertexIndex + j, originalArray->GetTuple(cellPoints[j]));
          }
        }
      }
    }

    newFaces->InsertNextCell(cellSize, cell.data());

    currentVertexIndex += cellSize;
  }

  output->SetPoints(newPoints);
  output->SetPolys(newFaces);

  return 1;
}
