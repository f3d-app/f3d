// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkF3DStochasticTransparentPass.h"

#include "vtkF3DRandomFS.h"

#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkF3DOpenGLGridMapper.h>

vtkStandardNewMacro(vtkF3DStochasticTransparentPass);
vtkCxxSetObjectMacro(vtkF3DStochasticTransparentPass, TranslucentPass, vtkRenderPass);
vtkCxxSetObjectMacro(vtkF3DStochasticTransparentPass, VolumetricPass, vtkRenderPass);

//------------------------------------------------------------------------------
vtkF3DStochasticTransparentPass::vtkF3DStochasticTransparentPass() = default;

//------------------------------------------------------------------------------
vtkF3DStochasticTransparentPass::~vtkF3DStochasticTransparentPass() = default;

//------------------------------------------------------------------------------
void vtkF3DStochasticTransparentPass::Render(const vtkRenderState* s)
{
  this->RenWin = static_cast<vtkOpenGLRenderWindow*>(s->GetRenderer()->GetRenderWindow());

  // Setup vtkOpenGLRenderPass
  this->PreRender(s);

  // force usage of depth buffer, even if the actors are translucents
  int numProps = s->GetPropArrayCount();
  for (int j = 0; j < numProps; ++j)
  {
    vtkProp* prop = s->GetPropArray()[j];
    vtkInformation* info = prop->GetPropertyKeys();
    if (!info)
    {
      info = vtkInformation::New();
      prop->SetPropertyKeys(info);
      info->FastDelete();
    }
    info->Set(vtkOpenGLActor::GLDepthMaskOverride(), 1);
  }

  this->TranslucentPass->Render(s);
  this->VolumetricPass->Render(s);

  this->PostRender(s);

  // remove depth buffer keys
  for (int j = 0; j < numProps; ++j)
  {
    vtkProp* prop = s->GetPropArray()[j];
    vtkInformation* info = prop->GetPropertyKeys();
    if (info)
    {
      info->Remove(vtkOpenGLActor::GLDepthMaskOverride());
    }
  }
}

//------------------------------------------------------------------------------
bool vtkF3DStochasticTransparentPass::SetShaderParameters(vtkShaderProgram* program,
  vtkAbstractMapper* mapper, vtkProp* prop, vtkOpenGLVertexArrayObject* VAO)
{
  // TODO: unused here, but blue noise should be better than white noise in theory
  program->SetUniformi("texNoise", this->RenWin->GetNoiseTextureUnit());

  return this->Superclass::SetShaderParameters(program, mapper, prop, VAO);
}

//------------------------------------------------------------------------------
bool vtkF3DStochasticTransparentPass::PreReplaceShaderValues(std::string& vertexShader,
  std::string& geometryShader, std::string& fragmentShader, vtkAbstractMapper* mapper,
  vtkProp* prop)
{
  if (!vtkF3DOpenGLGridMapper::SafeDownCast(mapper))
  {
    // add random function utilities
    std::string dec = vtkF3DRandomFS;
    dec += "\nuniform sampler2D texNoise;\n";

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Color::Dec", dec);

    vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Color::Impl",
      "  //VTK::Color::Impl\n"
      "  vec2 nsz = vec2(64, 64);\n"
      //"  float randomAngle = random(gl_PrimitiveID) * 6.28318530718;\n"
      //"  vec2 jitter = vec2(cos(randomAngle), sin(randomAngle));\n"
      //"  if (texture(texNoise, (gl_FragCoord.xy + jitter) / nsz).x >= opacity) discard;\n"
      //"  if (random(vec3(gl_FragCoord.xy, gl_PrimitiveID)) >= opacity) discard;\n"
      "  opacity = random(gl_FragCoord.x);\n\n");
  }

  return true;
}

//------------------------------------------------------------------------------
void vtkF3DStochasticTransparentPass::ReleaseGraphicsResources(vtkWindow* win)
{
}
