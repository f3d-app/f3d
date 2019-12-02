#include <cstdio>

#include "Config.h"

#include "F3DLoader.h"
#include "F3DLog.h"
#include "F3DOptions.h"
#include "F3DViewer.h"

int main(int argc, char** argv)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  F3DOptions options;
  F3DOptionsParser parser(options, argc, argv);

  F3DLoader loader(options);

  F3DViewer viewer(&options, loader.GetImporter());
  return viewer.Start();
}

#if F3D_WIN32_APP
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return main(__argc, __argv);
}
#endif
