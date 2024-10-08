#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicBackgroundColor(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.filename = true;
  opt.ui.filename_info = "(1/1) cow.vtp";

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  // Change the background color and make sure it is taken into account
  opt.render.background.color = { 1.0, 1.0, 1.0 };

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicBackgrounColor", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
