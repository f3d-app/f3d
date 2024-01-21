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
  vtkPointData* inputPointData = input->GetPointData();

  vtkIdType nbArrays = inputPointData->GetNumberOfArrays();

  bool earlyExit = true;

  for (vtkIdType i = 0; i < nbArrays; i++)
  {
    vtkDataArray* inputArray = inputPointData->GetArray(i);

    vtkInformation* info = inputArray->GetInformation();
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

  vtkPoints* inputPoints = input->GetPoints();
  vtkCellArray* inputFaces = input->GetPolys();

  vtkIdType nbCells = inputFaces->GetNumberOfCells();
  vtkIdType nbConnectivity = inputFaces->GetNumberOfConnectivityIds();

  // the number of output points is the number of total cells connectivity
  vtkNew<vtkPoints> outputPoints;
  outputPoints->SetNumberOfPoints(nbConnectivity);

  vtkPointData* outputPointData = output->GetPointData();

  // all point data attributes are copied so that face-varying attributes are not modified
  // since the output polydata will have the expected number of points
  outputPointData->ShallowCopy(inputPointData);

  // However, for vertex attributes, the arrays must be replaced by dispatching the values
  // in order to duplicate values and correspond to the new point location
  for (vtkIdType i = 0; i < nbArrays; i++)
  {
    vtkDataArray* inputArray = inputPointData->GetArray(i);

    vtkInformation* info = inputArray->GetInformation();
    int interpType = info->Get(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE());

    if (interpType == 0) // vertex
    {
      auto outputArray = vtkSmartPointer<vtkDataArray>::Take(
        vtkDataArray::CreateDataArray(inputArray->GetDataType()));
      outputArray->SetNumberOfComponents(inputArray->GetNumberOfComponents());
      outputArray->SetNumberOfTuples(nbConnectivity);
      outputArray->SetName(inputArray->GetName());

      outputPointData->AddArray(outputArray);
    }
  }

  vtkNew<vtkCellArray> outputFaces;

  std::vector<vtkIdType> cell;
  cell.resize(inputFaces->GetMaxCellSize());

  vtkIdType currentVertexIndex = 0;

  for (vtkIdType i = 0; i < nbCells; i++)
  {
    vtkIdType cellSize;
    const vtkIdType* cellPoints;
    inputFaces->GetCellAtId(i, cellSize, cellPoints);

    for (vtkIdType j = 0; j < cellSize; j++)
    {
      cell[j] = currentVertexIndex + j;
      outputPoints->SetPoint(currentVertexIndex + j, inputPoints->GetPoint(cellPoints[j]));

      for (vtkIdType k = 0; k < nbArrays; k++)
      {
        vtkDataArray* inputArray = inputPointData->GetArray(k);

        vtkInformation* info = inputArray->GetInformation();
        int interpType = info->Get(vtkF3DFaceVaryingPointDispatcher::INTERPOLATION_TYPE());

        if (interpType == 0) // vertex
        {
          vtkDataArray* outputArray = outputPointData->GetArray(inputArray->GetName());

          if (outputArray)
          {
            outputArray->SetTuple(currentVertexIndex + j, inputArray->GetTuple(cellPoints[j]));
          }
        }
      }
    }

    outputFaces->InsertNextCell(cellSize, cell.data());

    currentVertexIndex += cellSize;
  }

  output->SetPoints(outputPoints);
  output->SetPolys(outputFaces);

  return 1;
}
