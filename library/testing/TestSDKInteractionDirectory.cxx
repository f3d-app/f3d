#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

int TestSDKInteractionDirectory(int argc, char* argv[])
{
  // This test mimics TestInteractionDirectory.
  f3d::engine eng(
    f3d::engine::CREATE_WINDOW | f3d::engine::CREATE_INTERACTOR | f3d::engine::WINDOW_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 }).set("scalars", "");
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/mb");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();

  std::string filename = "TestSDKInteractionDirectory";
  inter.playInteraction(
    std::string(argv[1]) + "recordings/" + filename + ".log"); // Right;Right;Right;Left;Up;

  if (load.getCurrentFileIndex() != 2)
  {
    std::cerr << std::string("Expecting a file index of 2, got ") +
        std::to_string(load.getCurrentFileIndex()) +
        "." << std::endl;
    return EXIT_FAILURE;
  }

  load.loadFile(f3d::loader::LoadFileEnum::LOAD_FIRST);
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);

  if (load.getCurrentFileIndex() != 3)
  {
    std::cerr << std::string("Expecting a file index of 3, got ") +
        std::to_string(load.getCurrentFileIndex()) +
        "." << std::endl;
    return EXIT_FAILURE;
  }

  load.setCurrentFileIndex(2);
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  return TestSDKHelpers::RenderTest(
           win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename, 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
