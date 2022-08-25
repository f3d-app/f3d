#include <engine.h>
#include <loader.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicHDRI(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.set("ui.filename", true);

  load.addFile(std::string(argv[1]) + "/data/cow.vtp").loadFile();

  win.render();

  // Change the hdri and make sure it is taken into account
  opt.set("render.background.hdri", std::string(argv[1]) + "data/palermo_park_1k.hdr");

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicHDRI", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
