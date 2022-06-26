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
  opt.set("filename", true);

  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  win.render();

  // Change the hdri and make sure it is taken into account
  opt.set("hdri", std::string(argv[1]) + "data/palermo_park_1k.hdr");
  win.update();

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicHDRI", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
