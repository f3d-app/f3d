#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <iostream>

int TestSDKMultiOptions(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::options& opt = eng.getOptions();
  f3d::window& win = eng.getWindow();

  std::string rightFilename = "mb/mb_1_0.vtp";
  std::string leftFilename = "mb/mb_2_0.vtp";
  std::string left = std::string(argv[1]) + "data/" + leftFilename;
  std::string right = std::string(argv[1]) + "data/" + rightFilename;

  // Render one geometry with a render option
  load.loadGeometry(left);
  opt.set("render.show-edges", true);
  opt.set("render.grid.enable", true);
  opt.set("ui.metadata", true);
  win.render();

  // Add another geometry
  load.loadGeometry(right);

  // Check rendering is correct
  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKMultiOptions", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
