#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <window.h>

int TestSDKDropZone([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = true;
  opt.ui.drop_zone.custom_binds = "None+Drop Ctrl+O None+H";
  win.render();

  test("Render Drop Zone",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDropZone"));
  return test.result();
}
