#include <vtkObjectFactory.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#include <vtkglad/include/glad/egl.h>
#endif

#include "vtkF3DEGLRenderWindow.h"

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow::vtkF3DEGLRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow::~vtkF3DEGLRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DEGLRenderWindow* vtkF3DEGLRenderWindow::New()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
  // Load core egl functions
  if (gladLoaderLoadEGL(EGL_NO_DISPLAY) == 0)
  {
    return nullptr;
  }
#endif

  VTK_STANDARD_NEW_BODY(vtkF3DEGLRenderWindow);
}
