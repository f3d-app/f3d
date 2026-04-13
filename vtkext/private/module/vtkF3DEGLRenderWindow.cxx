#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#include <vtk_glad.h>
#include <vtkglad/include/glad/egl.h>

#include "vtkF3DEGLRenderWindow.h"

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow::vtkF3DEGLRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow::~vtkF3DEGLRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow* vtkF3DEGLRenderWindow::New()
{
  // Load core egl functions
  if (gladLoaderLoadEGL(EGL_NO_DISPLAY) == 0)
  {
    return nullptr;
  }

  VTK_STANDARD_NEW_BODY(vtkF3DEGLRenderWindow);
}
