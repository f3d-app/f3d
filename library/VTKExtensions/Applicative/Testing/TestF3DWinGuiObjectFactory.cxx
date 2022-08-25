#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DConfigure.h"
#include "vtkF3DObjectFactory.h"
#include "vtkF3DWin32OutputWindow.h"

int TestF3DWinGuiObjectFactory(int argc, char* argv[])
{
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  vtkNew<vtkOutputWindow> window;
  vtkF3DWin32OutputWindow* windowPtr = vtkF3DWin32OutputWindow::SafeDownCast(window);
  if (windowPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create a vtkF3DWin32OutputWindow" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
