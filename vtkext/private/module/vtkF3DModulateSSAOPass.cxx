#include "vtkF3DModulateSSAOPass.h"

#include <vtkObjectFactory.h>
#include <vtkRenderState.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

#include <cassert>

vtkStandardNewMacro(vtkF3DModulateSSAOPass);

// ----------------------------------------------------------------------------
void vtkF3DModulateSSAOPass::Render(const vtkRenderState* s)
{
  assert(this->DelegatePass);
  assert(this->SSAOTexture);
  assert(this->SSAOTexture->GetHandle());

  this->SSAOTexture->Activate();
  this->Superclass::Render(s);
  this->SSAOTexture->Deactivate();
}

// ----------------------------------------------------------------------------
bool vtkF3DModulateSSAOPass::PostReplaceShaderValues(
  std::string&, std::string&, std::string& fragmentShader, vtkAbstractMapper*, vtkProp*)
{
  vtkShaderProgram::Substitute(fragmentShader, "//VTK::System::Dec",
    "//VTK::System::Dec\n"
    "uniform sampler2D texSSAO;\n",
    false);

  vtkShaderProgram::Substitute(fragmentShader, "//VTK::Picking::Impl",
    "  ivec2 ssaoPixel = min(ivec2(gl_FragCoord.xy), textureSize(texSSAO, 0) - 1);\n"
    "  gl_FragData[0].rgb *= texelFetch(texSSAO, ssaoPixel, 0).r;\n  //VTK::Picking::Impl",
    false);

  return true;
}

// ----------------------------------------------------------------------------
bool vtkF3DModulateSSAOPass::SetShaderParameters(
  vtkShaderProgram* program, vtkAbstractMapper*, vtkProp*, vtkOpenGLVertexArrayObject*)
{
  assert(this->SSAOTexture);
  assert(this->SSAOTexture->GetHandle());

  program->SetUniformi("texSSAO", this->SSAOTexture->GetTextureUnit());
  return true;
}
