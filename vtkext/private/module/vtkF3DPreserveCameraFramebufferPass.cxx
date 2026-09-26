#include "vtkF3DPreserveCameraFramebufferPass.h"

#include <vtkCamera.h>
#include <vtkDepthImageProcessingPass.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPixelBufferObject.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DPreserveCameraFramebufferPass);

void vtkF3DPreserveCameraFramebufferPass::RenderDelegate(const vtkRenderState* s, int width,
  int height, int vtkNotUsed(newWidth), int vtkNotUsed(newHeight), vtkOpenGLFramebufferObject* fbo,
  vtkTextureObject* colortarget, vtkTextureObject* depthtarget)
{
  // Adapted from `vtkFramebufferPass::RenderDelegate`
  // Copyright (c) Kitware, Inc.
  vtkRenderer* r = s->GetRenderer();
  vtkRenderState s2(r);
  s2.SetPropArrayAndCount(s->GetPropArray(), s->GetPropArrayCount());

  s2.SetFrameBuffer(fbo);
  fbo->Bind();
  fbo->AddColorAttachment(0, colortarget);

  // because the same FBO can be used in another pass but with several color
  // buffers, force this pass to use 1, to avoid side effects from the
  // render of the previous frame.
  fbo->ActivateDrawBuffer(0);

  fbo->AddDepthAttachment(depthtarget);
  fbo->StartNonOrtho(width, height);

  // 2. Delegate render in FBO
  this->DelegatePass->Render(&s2);

  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();
}
