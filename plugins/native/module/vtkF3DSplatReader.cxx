#include "vtkF3DSplatReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DSplatReader);

//----------------------------------------------------------------------------
vtkF3DSplatReader::vtkF3DSplatReader()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkF3DSplatReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::ifstream inputStream(this->FileName, std::ios::binary);

  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});

  // position: 3 floats (12 bytes)
  // scale: 3 floats (12 bytes)
  // rotation: 4 chars (4 bytes)
  // color+opacity: 4 chars (4 bytes)
  constexpr size_t splatSize = 32;
  constexpr size_t positionOffset = 0;
  constexpr size_t scaleOffset = 12;
  constexpr size_t colorOffset = 24;
  constexpr size_t rotationOffset = 28;

  size_t nbSplats = buffer.size() / splatSize;

  vtkNew<vtkFloatArray> positionArray;
  positionArray->SetNumberOfComponents(3);
  positionArray->SetNumberOfTuples(nbSplats);
  positionArray->SetName("position");

  vtkNew<vtkFloatArray> scaleArray;
  scaleArray->SetNumberOfComponents(3);
  scaleArray->SetNumberOfTuples(nbSplats);
  scaleArray->SetName("scale");

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(nbSplats);
  colorArray->SetName("color");

  vtkNew<vtkFloatArray> rotationArray;
  rotationArray->SetNumberOfComponents(4);
  rotationArray->SetNumberOfTuples(nbSplats);
  rotationArray->SetName("rotation");

  for (size_t i = 0; i < nbSplats; i++)
  {
    float* position = reinterpret_cast<float*>(buffer.data() + (splatSize * i) + positionOffset);
    float* scale = reinterpret_cast<float*>(buffer.data() + (splatSize * i) + scaleOffset);
    unsigned char* rotation = buffer.data() + (splatSize * i) + rotationOffset;
    unsigned char* color = buffer.data() + (splatSize * i) + colorOffset;

    positionArray->SetTypedTuple(i, position);
    scaleArray->SetTypedTuple(i, scale);
    colorArray->SetTypedTuple(i, color);

    for (int c = 0; c < 4; c++)
    {
      rotationArray->SetTypedComponent(i, c, (static_cast<float>(rotation[c]) - 128.f) / 128.f);
    }
  }

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetData(positionArray);
  output->SetPoints(points);

  output->GetPointData()->SetScalars(colorArray);
  output->GetPointData()->AddArray(scaleArray);
  output->GetPointData()->AddArray(rotationArray);

  return 1;
}
