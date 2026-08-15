#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

int TestSDKDynamicFontFile([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
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

  test("Render with dynamic font file",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicFontFile"));
  return test.result();
}
