#include <engine.h>
#include <interactor.h>
#include <options.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

int TestSDKInteractorDropFullScene([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  std::ignore = eng.getOptions();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  win.setSize(300, 300);
  win.render();

  std::string filename = "TestSDKInteractorDropFullScene";
  std::string interactionFilePath = std::string(argv[2]) + "../../" + filename + ".log";
  inter.playInteraction(interactionFilePath); // world.obj; S

  return TestSDKHelpers::RenderTest(
           win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
