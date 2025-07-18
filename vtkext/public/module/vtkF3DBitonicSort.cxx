#include "vtkF3DBitonicSort.h"

#include "vtkF3DBitonicSortFunctions.h"
#include "vtkF3DBitonicSortGlobalDisperseCS.h"
#include "vtkF3DBitonicSortGlobalFlipCS.h"
#include "vtkF3DBitonicSortLocalDisperseCS.h"
#include "vtkF3DBitonicSortLocalSortCS.h"

#include <vtkObjectFactory.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DBitonicSort);

//----------------------------------------------------------------------------
bool vtkF3DBitonicSort::Initialize(int workgroupSize, int keyType, int valueType)
{
  if (workgroupSize <= 0)
  {
    vtkErrorMacro("Invalid workgroupSize");
    return false;
  }

  auto GetStringShaderType = [](int vtkType) -> std::string
  {
    switch (vtkType)
    {
      case VTK_INT:
        return "int";
      case VTK_UNSIGNED_INT:
        return "uint";
      case VTK_FLOAT:
        return "float";
      case VTK_DOUBLE:
        return "double";
    }
    return "";
  };

  std::string keyTypeShader = GetStringShaderType(keyType);
  if (keyTypeShader.empty())
  {
    vtkErrorMacro("Invalid keyType");
    return false;
  }

  std::string valueTypeShader = GetStringShaderType(valueType);
  if (valueTypeShader.empty())
  {
    vtkErrorMacro("Invalid valueType");
    return false;
  }

  std::stringstream defines;
  defines << "#define KeyType " << keyTypeShader << "\n";
  defines << "#define ValueType " << valueTypeShader << "\n";
  defines << "#define WorkgroupSize " << workgroupSize << "\n";

  std::string localSort = vtkF3DBitonicSortLocalSortCS;
  vtkShaderProgram::Substitute(
    localSort, "//VTK::BitonicFunctions::Dec", vtkF3DBitonicSortFunctions);
  vtkShaderProgram::Substitute(localSort, "//VTK::BitonicDefines::Dec", defines.str());

  std::string localDisperse = vtkF3DBitonicSortLocalDisperseCS;
  vtkShaderProgram::Substitute(
    localDisperse, "//VTK::BitonicFunctions::Dec", vtkF3DBitonicSortFunctions);
  vtkShaderProgram::Substitute(localDisperse, "//VTK::BitonicDefines::Dec", defines.str());

  std::string globalDisperse = vtkF3DBitonicSortGlobalDisperseCS;
  vtkShaderProgram::Substitute(
    globalDisperse, "//VTK::BitonicFunctions::Dec", vtkF3DBitonicSortFunctions);
  vtkShaderProgram::Substitute(globalDisperse, "//VTK::BitonicDefines::Dec", defines.str());

  std::string globalFlip = vtkF3DBitonicSortGlobalFlipCS;
  vtkShaderProgram::Substitute(
    globalFlip, "//VTK::BitonicFunctions::Dec", vtkF3DBitonicSortFunctions);
  vtkShaderProgram::Substitute(globalFlip, "//VTK::BitonicDefines::Dec", defines.str());

  this->BitonicSortLocalSortComputeShader->SetType(vtkShader::Compute);
  this->BitonicSortLocalSortComputeShader->SetSource(localSort);
  this->BitonicSortLocalSortProgram->SetComputeShader(this->BitonicSortLocalSortComputeShader);

  this->BitonicSortLocalDisperseComputeShader->SetType(vtkShader::Compute);
  this->BitonicSortLocalDisperseComputeShader->SetSource(localDisperse);
  this->BitonicSortLocalDisperseProgram->SetComputeShader(
    this->BitonicSortLocalDisperseComputeShader);

  this->BitonicSortGlobalDisperseComputeShader->SetType(vtkShader::Compute);
  this->BitonicSortGlobalDisperseComputeShader->SetSource(globalDisperse);
  this->BitonicSortGlobalDisperseProgram->SetComputeShader(
    this->BitonicSortGlobalDisperseComputeShader);

  this->BitonicSortGlobalFlipComputeShader->SetType(vtkShader::Compute);
  this->BitonicSortGlobalFlipComputeShader->SetSource(globalFlip);
  this->BitonicSortGlobalFlipProgram->SetComputeShader(this->BitonicSortGlobalFlipComputeShader);

  this->WorkgroupSize = workgroupSize;

  return true;
}

//----------------------------------------------------------------------------
bool vtkF3DBitonicSort::Run(vtkOpenGLRenderWindow* context, int nbPairs,
  vtkOpenGLBufferObject* keys, vtkOpenGLBufferObject* values)
{
  if (this->WorkgroupSize <= 0)
  {
    vtkErrorMacro("Shaders are not initialized");
    return false;
  }

  vtkOpenGLShaderCache* shaderCache = context->GetShaderCache();

  // compute next power of two
  unsigned int nbPairsExt = vtkMath::NearestPowerOfTwo(nbPairs);

  const int workgroupCount = std::max(nbPairsExt / (this->WorkgroupSize * 2), 1U);

  keys->BindShaderStorage(0);
  values->BindShaderStorage(1);

  // first, sort all workgroups locally
  shaderCache->ReadyShaderProgram(this->BitonicSortLocalSortProgram);
  this->BitonicSortLocalSortProgram->SetUniformi("count", nbPairs);
  glDispatchCompute(workgroupCount, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // we must now double h, as this happens before every flip
  for (unsigned int outerHeight = this->WorkgroupSize * 2; outerHeight < nbPairsExt;
       outerHeight *= 2)
  {
    shaderCache->ReadyShaderProgram(this->BitonicSortGlobalFlipProgram);
    this->BitonicSortGlobalFlipProgram->SetUniformi("count", nbPairs);
    this->BitonicSortGlobalFlipProgram->SetUniformi("height", outerHeight);
    glDispatchCompute(workgroupCount, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    for (int innerHeight = outerHeight / 2; innerHeight > this->WorkgroupSize; innerHeight /= 2)
    {
      shaderCache->ReadyShaderProgram(this->BitonicSortGlobalDisperseProgram);
      this->BitonicSortGlobalDisperseProgram->SetUniformi("count", nbPairs);
      this->BitonicSortGlobalDisperseProgram->SetUniformi("height", innerHeight);
      glDispatchCompute(workgroupCount, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // handle the remaining disperse loop locally to the workgroup
    shaderCache->ReadyShaderProgram(this->BitonicSortLocalDisperseProgram);
    this->BitonicSortLocalDisperseProgram->SetUniformi("count", nbPairs);
    glDispatchCompute(workgroupCount, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  return true;
}
