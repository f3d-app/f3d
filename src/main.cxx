#include "F3DLoader.h"
#include "Config.h"

int main(int argc, char** argv)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

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
