#include "vtkF3DSplatReader.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkResourceStream.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>

#include <iostream>

namespace
{
  struct splat_t
  {
    float position[3];
    float scale[3];
    unsigned char color[4];
    unsigned char rotation[4];
  };
}

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

  vtkSmartPointer<vtkResourceStream> stream;

#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 4, 20250501)
  if (this->Stream)
  {
    stream = this->Stream;
    assert(this->Stream->SupportSeek());
  }
  else
#endif
  {
    vtkNew<vtkFileResourceStream> fileStream;
    fileStream->Open(this->FileName);
    stream = fileStream;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  std::size_t nbSplats = stream->Tell() / sizeof(::splat_t);
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);

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

  ::splat_t splat;
  for (size_t i = 0; i < nbSplats; i++)
  {
    if (stream->Read(&splat, sizeof(::splat_t)) != sizeof(::splat_t))
    {
      vtkWarningMacro("Could not read a splat at index: " << i << " , result may be incorrect");
      break;
    }

    positionArray->SetTypedTuple(i, splat.position);
    scaleArray->SetTypedTuple(i, splat.scale);
    colorArray->SetTypedTuple(i, splat.color);
    for (int c = 0; c < 4; c++)
    {
      rotationArray->SetTypedComponent(i, c, (static_cast<float>(splat.rotation[c]) - 128.f) / 128.f);
    }

    // TODO This should always be 1 according to math?
    float squareSum = 0;
    for (int c = 0; c < 4; c++)
    {
      float rot = (static_cast<float>(splat.rotation[c]) - 128.f) / 128.f;
      squareSum += rot * rot;
    }
    std::cout<<"squareSum: " << squareSum<<std::endl;

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

//------------------------------------------------------------------------------
bool vtkF3DSplatReader::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  // Check the size of the file
  stream->Seek(0, vtkResourceStream::SeekDirection::End);
  vtkTypeInt64 streamSize = stream->Tell();
  if (streamSize % sizeof(::splat_t) != 0)
  {
    return false;
  }


  // Read the first splat
  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  ::splat_t splat;
  if (stream->Read(&splat, sizeof(::splat_t)) != sizeof(::splat_t))
  {
    return false;
  }

  // Check first splat scale is all positive
  for (int axis = 0; axis < 3; axis++)
  {
    if (splat.scale[axis] < 0)
    {
      return false;
    }
  }

  // Check sum of squared rotation is 1
  // TODO check assumption is correct
  float squareSum = 0;
  for (int c = 0; c < 4; c++)
  {
    float rot = (static_cast<float>(splat.rotation[c]) - 128.f) / 128.f;
    squareSum += rot * rot;
  }
/*  if (squareSum != 1)
  {
    return false;
  }*/

  return true;
}
