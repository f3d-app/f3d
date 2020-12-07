#include "vtkF3DWin32OutputWindow.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DWin32OutputWindow);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
const char* vtkF3DWin32OutputWindow::GetWindowTitle()
{
  return "F3D log window";
}
#endif
