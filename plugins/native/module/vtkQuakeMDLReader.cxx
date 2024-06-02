#include "vtkQuakeMDLReader.h"

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
vtkStandardNewMacro(vtkQuakeMDLReader);

//----------------------------------------------------------------------------
vtkQuakeMDLReader::vtkQuakeMDLReader()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkQuakeMDLReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);

  std::ifstream inputStream(this->FileName, std::ios::binary);

  // identity ("IDPO"): 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> IDPOArray;
  IDPOArray->SetNumberOfComponents(4);
  IDPOArray->SetName("identity");

  // version: 1 int (4 bytes)
  vtkNew<vtkIntArray> version;
  version->SetNumberOfComponents(1);
  version->SetName("version");

  //====================================

  // scaling factor: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> scalingFactor;
  scalingFactor->SetNumberOfComponents(3);
  scalingFactor->SetName("scaling factor");

  // translation vector: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> translationVector;
  translationVector->SetNumberOfComponents(3);
  translationVector->SetName("translation vector");

  // bounding radius: 1 float (4 bytes)
  vtkNew<vtkFloatArray> boundingRadius;
  boundingRadius->SetNumberOfComponents(1);
  boundingRadius->SetName("bounding radius");

  // eye position: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> eyePosition;
  eyePosition->SetNumberOfComponents(3);
  eyePosition->SetName("eye position");

  //====================================

  // number of textures: 1 int (4 bytes)
  vtkNew<vtkIntArray> texturesNum;
  texturesNum->SetNumberOfComponents(1);
  texturesNum->SetName("number of textures");

  // texture width: 1 int (4 bytes)
  vtkNew<vtkIntArray> textureWidth;
  textureWidth->SetNumberOfComponents(1);
  textureWidth->SetName("texture width");

  // texture height: 1 int (4 bytes)
  vtkNew<vtkIntArray> textureHeight;
  textureHeight->SetNumberOfComponents(1);
  textureHeight->SetName("texture height");

  //====================================

  // number of vertices: 1 int (4 bytes)
  vtkNew<vtkIntArray> verticesNum;
  verticesNum->SetNumberOfComponents(1);
  verticesNum->SetName("number of vertices");

  // number of triangles: 1 int (4 bytes)
  vtkNew<vtkIntArray> trianglesNum;
  trianglesNum->SetNumberOfComponents(1);
  trianglesNum->SetName("number of triangles");

  // number of frames: 1 int (4 bytes)
  vtkNew<vtkIntArray> framesNum;
  framesNum->SetNumberOfComponents(1);
  framesNum->SetName("number of frames");

  //====================================

  // sync type (0: synchron, 1: random): 1 int (4 bytes)
  vtkNew<vtkIntArray> syncType;
  syncType->SetNumberOfComponents(1);
  syncType->SetName("sync type");

  // state flags: 1 int (4 bytes)
  vtkNew<vtkIntArray> stateFlags;
  stateFlags->SetNumberOfComponents(1);
  stateFlags->SetName("state flags");

  //====================================

  // position: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> position;
  position->SetNumberOfComponents(3);
  position->SetName("position");

  // scale: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> scale;
  scale->SetNumberOfComponents(3);
  scale->SetName("scale");

  // rotation: 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> rotation;
  rotation->SetNumberOfComponents(4);
  rotation->SetName("rotation");

  // color+opacity: 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> colorAndOpacity;
  colorAndOpacity->SetNumberOfComponents(4);
  colorAndOpacity->SetName("color and opacity");

  return 1;
}
