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

  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});

  //??? What's a “splat”???
  constexpr size_t splatSize = 32;

  //??? What is nbSplats???
  size_t nbSplats = buffer.size() / splatSize;

  // identity ("IDPO"): 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> IDPOArray;
  IDPOArray->SetNumberOfComponents(4);
  IDPOArray->SetNumberOfTuples(nbSplats);
  IDPOArray->SetName("identity");

  // version: 1 int (4 bytes)
  vtkNew<vtkIntArray> version;
  version->SetNumberOfComponents(1);
  version->SetNumberOfTuples(nbSplats);
  version->SetName("version");

  //====================================

  // scaling factor: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> scalingFactor;
  scalingFactor->SetNumberOfComponents(3);
  scalingFactor->SetNumberOfTuples(nbSplats);
  scalingFactor->SetName("scaling factor");

  // translation vector: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> translationVector;
  translationVector->SetNumberOfComponents(3);
  translationVector->SetNumberOfTuples(nbSplats);
  translationVector->SetName("translation vector");

  // bounding radius: 1 float (4 bytes)
  vtkNew<vtkFloatArray> boundingRadius;
  boundingRadius->SetNumberOfComponents(1);
  boundingRadius->SetNumberOfTuples(nbSplats);
  boundingRadius->SetName("bounding radius");

  // eye position: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> eyePosition;
  eyePosition->SetNumberOfComponents(3);
  eyePosition->SetNumberOfTuples(nbSplats);
  eyePosition->SetName("eye position");

  //====================================
  
  // number of textures: 1 int (4 bytes)
  vtkNew<vtkIntArray> texturesNum;
  texturesNum->SetNumberOfComponents(1);
  texturesNum->SetNumberOfTuples(nbSplats);
  texturesNum->SetName("number of textures");
  
  // texture width: 1 int (4 bytes)
  vtkNew<vtkIntArray> textureWidth;
  textureWidth->SetNumberOfComponents(1);
  texturesWidth->SetNumberOfTuples(nbSplats);
  textureWidth->SetName("texture width");
  
  // texture height: 1 int (4 bytes)
  vtkNew<vtkIntArray> textureHeight;
  textureHeight->SetNumberOfComponents(1);
  textureHeight->SetNumberOfTuples(nbSplats);
  textureHeight->SetName("texture height");

  //====================================

  // number of vertices: 1 int (4 bytes)
  vtkNew<vtkIntArray> verticesNum;
  verticesNum->SetNumberOfComponents(1);
  verticesNum->SetNumberOfTuples(nbSplats);
  verticesNum->SetName("number of vertices");

  // number of triangles: 1 int (4 bytes)
  vtkNew<vtkIntArray> trianglesNum;
  trianglesNum->SetNumberOfComponents(1);
  trianglesNum->SetNumberOfTuples(nbSplats);
  trianglesNum->SetName("number of triangles");

  // number of frames: 1 int (4 bytes)
  vtkNew<vtkIntArray> framesNum;
  framesNum->SetNumberOfComponents(1);
  framesNum->SetNumberOfTuples(nbSplats);
  framesNum->SetName("number of frames");

  //====================================

  // sync type (0: synchron, 1: random): 1 int (4 bytes)
  vtkNew<vtkIntArray> syncType;
  syncType->SetNumberOfComponents(1);
  syncType->SetNumberOfTuples(nbSplats);
  syncType->SetName("sync type");

  // state flags: 1 int (4 bytes)
  vtkNew<vtkIntArray> stateFlags;
  stateFlags->SetNumberOfComponents(1);
  stateFlags->SetNumberOfTuples(nbSplats);
  stateFlags->SetName("state flags");

  //====================================

  // position: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> position;
  position->SetNumberOfComponents(3);
  position->SetNumberOfTuples(nbSplats);
  position->SetName("position");

  // scale: 3 floats (12 bytes)
  vtkNew<vtkFloatArray> scale;
  scale->SetNumberOfComponents(3);
  scale->SetNumberOfTuples(nbSplats);
  scale->SetName("scale");

  // rotation: 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> rotation;
  rotation->SetNumberOfComponents(4);
  rotation->SetNumberOfTuples(nbSplats);
  rotation->SetName("rotation");

  // color+opacity: 4 chars (4 bytes)
  vtkNew<vtkUnsignedCharArray> colorAndOpacity;
  colorAndOpacity->SetNumberOfComponents(4);
  colorAndOpacity->SetNumberOfTuples(nbSplats);
  colorAndOpacity->SetName("color and opacity");

  return 1;
}
