#include "vtkF3DConfigure.h"
#include "vtkF3DObjectFactory.h"
#include "vtkF3DPolyDataMapper.h"

#if F3D_WINDOWS_GUI
#include "vtkF3DWin32OutputWindow.h"
#else
#include "vtkF3DConsoleOutputWindow.h"
#endif

#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkTestUtilities.h>

int TestF3DObjectFactory(int argc, char* argv[])
{
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  vtkNew<vtkPolyDataMapper> mapper;
  vtkF3DPolyDataMapper* mapperPtr = vtkF3DPolyDataMapper::SafeDownCast(mapper);
  if (mapperPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create a vtkF3DPolyDataMapper" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkOutputWindow> window;
#if F3D_WINDOWS_GUI
  vtkF3DWin32OutputWindow* windowPtr = vtkF3DWin32OutputWindow::SafeDownCast(window);
#else
  vtkF3DConsoleOutputWindow* windowPtr = vtkF3DConsoleOutputWindow::SafeDownCast(window);
#endif
  if (windowPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create the correct type of vtkOutputWindow"
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
