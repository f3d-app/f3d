#include "vtkF3DTAAResolvePass.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderUtilities.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"

vtkStandardNewMacro(vtkF3dTAAResolvePass);

//------------------------------------------------------------------------------
void vtkF3dTAAResolvePass::Render(const vtkRenderState* s)
{
  // vtkOpenGLClearErrorMacro();

  // vtkRenderer* renderer = s->GetRenderer();
}

//------------------------------------------------------------------------------
void vtkF3dTAAResolvePass::ReleaseGraphicsResources(vtkWindow* w)
{
  // this->Superclass::ReleaseGraphicsResources(w);

  // if (this->FrameBufferObject)
  // {
  //   this->FrameBufferObject->ReleaseGraphicsResources(w);
  // }
  // if (this->ColorTexture)
  // {
  //   this->ColorTexture->ReleaseGraphicsResources(w);
  // }
  // this->QuadHelper = nullptr;
}