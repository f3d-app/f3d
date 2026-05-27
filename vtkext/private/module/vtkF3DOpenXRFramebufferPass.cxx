#include "vtkF3DOpenXRFramebufferPass.h"

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

#if F3D_MODULE_OPENXR
#include <vtkOpenXRCamera.h>
#endif

#include <cassert>

vtkStandardNewMacro(vtkF3DOpenXRFramebufferPass);

void vtkF3DOpenXRFramebufferPass::RenderDelegate(const vtkRenderState* s, int width, int height,
  int vtkNotUsed(newWidth), int vtkNotUsed(newHeight), vtkOpenGLFramebufferObject* fbo,
  vtkTextureObject* colortarget, vtkTextureObject* depthtarget)
{
#if F3D_MODULE_OPENXR
  assert("pre: s_exists" && s != nullptr);
  assert("pre: fbo_exists" && fbo != nullptr);
  assert("pre: fbo_has_context" && fbo->GetContext() != nullptr);
  assert("pre: colortarget_exists" && colortarget != nullptr);
  assert("pre: colortarget_has_context" && colortarget->GetContext() != nullptr);
  assert("pre: depthtarget_exists" && depthtarget != nullptr);
  assert("pre: depthtarget_has_context" && depthtarget->GetContext() != nullptr);

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
  // glEnable(GL_DEPTH_TEST);
  this->DelegatePass->Render(&s2);

  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();
#else
  assert(false); // Unreachable
#endif
}
