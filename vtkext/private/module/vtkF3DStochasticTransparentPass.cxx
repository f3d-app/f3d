// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkF3DStochasticTransparentPass.h"

#include <vtkObjectFactory.h>
#include <vtkRenderState.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderer.h>

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
  vtkOpenGLRenderWindow* renWin =
    static_cast<vtkOpenGLRenderWindow*>(s->GetRenderer()->GetRenderWindow());

  // Setup vtkOpenGLRenderPass
  this->PreRender(s);

  this->TranslucentPass->Render(s);
  this->VolumetricPass->Render(s);

  this->PostRender(s);
}

//------------------------------------------------------------------------------
void vtkF3DStochasticTransparentPass::ReleaseGraphicsResources(vtkWindow* win)
{
}
