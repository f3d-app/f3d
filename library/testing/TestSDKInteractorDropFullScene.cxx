#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

int TestSDKInteractorDropFullScene(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  win.setSize(300, 300);

  std::string filename = "TestSDKInteractorDropFullScene";
#if defined(_WIN32)
  std::string interactionFilePath = std::string(argv[2]) + "..\\..\\" + filename + ".log";
#else
  std::string interactionFilePath = std::string(argv[2]) + "../../" + filename + ".log";
#endif

  inter.playInteraction(interactionFilePath); // world.obj; S

  return TestSDKHelpers::RenderTest(
           win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename, 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
