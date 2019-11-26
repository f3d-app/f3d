#include <cstdio>

#include "Config.h"

#include "F3DLoader.h"
#include "F3DLog.h"
#include "F3DOptions.h"
#include "F3DViewer.h"

int main(int argc, char** argv)
{
  F3DOptions options;
  options.InitializeFromArgs(argc, argv);

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
