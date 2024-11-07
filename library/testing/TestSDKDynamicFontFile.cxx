#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicFontFile(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.filename = true;
  opt.ui.filename_info = "(1/1) cow.vtp";

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  // Change the font file and make sure it is taken into account
  opt.ui.font_file = std::string(argv[1]) + "data/Crosterian.ttf";

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicFontFile")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
