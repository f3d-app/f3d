#include <engine.h>
#include <interactor.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDropZoneNoLogo([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = false;
  opt.ui.drop_zone.custom_binds = "None+Drop Ctrl+O None+H";

  win.render();

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDropZoneNoLogo")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
