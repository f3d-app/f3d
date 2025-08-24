#include <vtkObjectFactory.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLState.h>

#include "vtkF3DExternalRenderWindow.h"

vtkStandardNewMacro(vtkF3DExternalRenderWindow);

//------------------------------------------------------------------------------
vtkF3DExternalRenderWindow::vtkF3DExternalRenderWindow()
{
  this->FrameBlitMode = BlitToCurrent;
}

//------------------------------------------------------------------------------
vtkF3DExternalRenderWindow::~vtkF3DExternalRenderWindow() = default;

//------------------------------------------------------------------------------
void vtkF3DExternalRenderWindow::Start()
{
  // creates or resizes the framebuffer
  this->Size[0] = (this->Size[0] > 0 ? this->Size[0] : 300);
  this->Size[1] = (this->Size[1] > 0 ? this->Size[1] : 300);
  this->CreateFramebuffers(this->Size[0], this->Size[1]);

  this->GetState()->PushFramebufferBindings();

  this->BlitToRenderFramebuffer(true);
  this->RenderFramebuffer->Bind();
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderWindow::Render()
{
  if (!this->Initialized)
  {
    // skip vtkGenericOpenGLRenderWindow because it calls OpenGL before it's even initialized
    this->vtkOpenGLRenderWindow::OpenGLInit();

    if (!this->Initialized)
    {
      // The context can fails to initialize if the OpenGL symbol loader is invalid.
      return;
    }
  }

  this->Superclass::Render();
}
