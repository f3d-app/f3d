#include "vtkF3DObjectFactory.h"
#include "vtkF3DPolyDataMapper.h"

#if F3D_MODULE_UI
#include "vtkF3DImguiConsole.h"
#else
#include "vtkF3DConsoleOutputWindow.h"
#endif

#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkTestUtilities.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240203)
#include "vtkF3DPointSplatMapper.h"
#endif

int TestF3DObjectFactory(int argc, char* argv[])
{
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240203)
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPointGaussianMapper", "vtkOpenGLPointGaussianMapper");
#endif

  // Check factory utility methods
  if (strcmp(factory->GetVTKSourceVersion(), VTK_SOURCE_VERSION) != 0)
  {
    std::cerr << "vtkF3DObjectFactory failed to provide correct VTK_SOURCE_VERSION\n";
    return EXIT_FAILURE;
  }
  if (strcmp(factory->GetDescription(), "F3D factory overrides.") != 0)
  {
    std::cerr << "vtkF3DObjectFactory failed to provide expected description\n";
    return EXIT_FAILURE;
  }

  // Check actual factory mechanism
  vtkNew<vtkPolyDataMapper> mapper;
  const vtkF3DPolyDataMapper* mapperPtr = vtkF3DPolyDataMapper::SafeDownCast(mapper);
  if (mapperPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create a vtkF3DPolyDataMapper\n";
    return EXIT_FAILURE;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240203)
  vtkNew<vtkPointGaussianMapper> pointMapper;
  pointMapper->Print(cout);
  vtkF3DPointSplatMapper* pointMapperPtr = vtkF3DPointSplatMapper::SafeDownCast(pointMapper);
  if (pointMapperPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create a vtkF3DPointSplatMapper\n";
    return EXIT_FAILURE;
  }
#endif

  vtkNew<vtkOutputWindow> window;
#if F3D_MODULE_UI
  const vtkF3DImguiConsole* windowPtr = vtkF3DImguiConsole::SafeDownCast(window);
#else
  vtkF3DConsoleOutputWindow* windowPtr = vtkF3DConsoleOutputWindow::SafeDownCast(window);
#endif
  if (windowPtr == nullptr)
  {
    std::cerr << "vtkF3DObjectFactory failed to create the correct type of vtkOutputWindow\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
