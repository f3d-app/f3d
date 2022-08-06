#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicFontFile(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.set("ui.filename", true);

  load.addFile(std::string(argv[1]) + "/data/cow.vtp").loadFile();

  win.render();

  // Change the font file and make sure it is taken into account
  opt.set("ui.font-file", std::string(argv[1]) + "data/AttackGraffiti-3zRBM.ttf");

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicFontFile", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
