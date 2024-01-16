#include "vtkF3DFaceVaryingPointDispatcher.h"

#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkF3DFaceVaryingPointDispatcher);

vtkInformationKeyMacro(vtkF3DFaceVaryingPointDispatcher, INTERPOLATION_TYPE, Integer);

//------------------------------------------------------------------------------
vtkF3DFaceVaryingPointDispatcher::vtkF3DFaceVaryingPointDispatcher() = default;

//------------------------------------------------------------------------------
vtkF3DFaceVaryingPointDispatcher::~vtkF3DFaceVaryingPointDispatcher() = default;

//------------------------------------------------------------------------------
int vtkF3DFaceVaryingPointDispatcher::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]->GetInformationObject(0));
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  // early exit if all interpolations are "vertex"
  vtkPointData* originalPointData = input->GetPointData();

  vtkIdType nbArrays = originalPointData->GetNumberOfArrays();

  bool earlyExit = true;

  for (vtkIdType i = 0; i < nbArrays; i++)
  {
    vtkDataArray* originalArray = originalPointData->GetArray(i);

    vtkInformation* info = originalArray->GetInformation();
    int interpType = info->Get(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE());

    if (interpType != 0) // vertex
    {
      earlyExit = false;
      break;
    }
  }

  if (earlyExit)
  {
    // nothing to do, just return the input
    output->ShallowCopy(input);
    return 1;
  }

  vtkPoints* originalPoints = input->GetPoints();
  vtkCellArray* originalFaces = input->GetPolys();

  vtkNew<vtkPoints> newPoints;
  vtkNew<vtkCellArray> newFaces;

  vtkIdType nbCells = originalFaces->GetNumberOfCells();
  vtkIdType nbConnectivity = originalFaces->GetNumberOfConnectivityIds();

  // the number of output points is the number of total cells connectivity
  newPoints->SetNumberOfPoints(nbConnectivity);

  vtkPointData* newPointData = output->GetPointData();

  // Use the interpolation type metadata to know if the array can be shallow copied
  // And initialize the arrays
  for (vtkIdType i = 0; i < nbArrays; i++)
  {
    vtkDataArray* originalArray = originalPointData->GetArray(i);

    vtkInformation* info = originalArray->GetInformation();
    int interpType = info->Get(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE());

    if (interpType == 0) // vertex
    {
      vtkDataArray* newArray = vtkDataArray::CreateDataArray(originalArray->GetDataType());
      newArray->SetNumberOfComponents(originalArray->GetNumberOfComponents());
      newArray->SetNumberOfTuples(nbConnectivity);
      newArray->SetName(originalArray->GetName());

      newPointData->AddArray(newArray);
      newArray->Delete();
    }
    else
    {
      // if faceVarying, just copy
      newPointData->AddArray(originalArray);
    }
  }

  // copy attribute flags from input
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

      for (vtkIdType k = 0; k < nbArrays; k++)
      {
        vtkDataArray* originalArray = originalPointData->GetArray(k);

        vtkInformation* info = originalArray->GetInformation();
        int interpType = info->Get(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE());

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
