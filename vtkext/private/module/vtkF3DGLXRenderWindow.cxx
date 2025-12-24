#include <vtkObjectFactory.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#include <vtkglad/include/glad/glx.h>
#else
#include <vtk_glew.h>
#endif

#include "vtkF3DGLXRenderWindow.h"

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251009)
#include "vtkX11Functions.h"
#else
#include <X11/Xlib.h>
#endif

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow::vtkF3DGLXRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow::~vtkF3DGLXRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DGLXRenderWindow* vtkF3DGLXRenderWindow::New()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251009)
  // Check if the X display is available
  vtkX11FunctionsInitialize();

  if (vtkXOpenDisplay == nullptr || vtkXCloseDisplay == nullptr)
  {
    // libX11 cannot be open
    return nullptr;
  }

  Display* dpy = vtkXOpenDisplay(nullptr);
  if (dpy == nullptr)
  {
    // no display available
    return nullptr;
  }
  vtkXCloseDisplay(dpy);

  vtkX11FunctionsFinalize();
#else
  Display* dpy = XOpenDisplay(nullptr);
  if (dpy == nullptr)
  {
    return nullptr;
  }
  XCloseDisplay(dpy);
#endif

  VTK_STANDARD_NEW_BODY(vtkF3DGLXRenderWindow);
}
