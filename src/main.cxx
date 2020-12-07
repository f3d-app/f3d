#include "Config.h"
#include "F3DLoader.h"

#include "vtkF3DObjectFactory.h"

int main(int argc, char** argv)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  F3DLoader loader;
  return loader.Start(argc, argv);
}

#if F3D_WIN32_APP
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return main(__argc, __argv);
}
#endif
