#include <vtkObjectFactory.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#include <vtkglad/include/glad/glx.h>
#else
#include <vtk_glew.h>
#endif

#include "vtkF3DGLXRenderWindow.h"

#include <X11/Xlib.h>

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow::vtkF3DGLXRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow::~vtkF3DGLXRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow* vtkF3DGLXRenderWindow::New()
{
  // Check if the X display is available
  Display* dpy = XOpenDisplay(nullptr);
  if (dpy == nullptr)
  {
    return nullptr;
  }
  XCloseDisplay(dpy);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
  // Load core glx functions
  gladLoaderLoadGLX(nullptr, 0);
#endif

  VTK_STANDARD_NEW_BODY(vtkF3DGLXRenderWindow);
}
