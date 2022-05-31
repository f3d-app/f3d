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
#include <vtkVersion.h>

int TestF3DObjectFactory(int argc, char* argv[])
{
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  // Check factory utility methods
  if (strcmp(factory->GetVTKSourceVersion(), VTK_SOURCE_VERSION) != 0)
  {
    std::cerr << "vtkF3DObjectFactory failed to provide correct VTK_SOURCE_VERSION" << std::endl;
    return EXIT_FAILURE;
  }
  if (strcmp(factory->GetDescription(), "F3D factory overrides.") != 0)
  {
    std::cerr << "vtkF3DObjectFactory failed to provide expected description" << std::endl;
    return EXIT_FAILURE;
  }

  // Check actual factory mechanism
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
