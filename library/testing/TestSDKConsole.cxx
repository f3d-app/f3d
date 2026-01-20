#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <window.h>

int TestSDKConsole([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO);

  // test warning with no color
  f3d::log::setUseColoring(false);
  f3d::log::warn("Message in console");

  f3d::engine eng = f3d::engine::create(true);

  f3d::options& opt = eng.getOptions();
  opt.ui.console = true;

  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  test("Render text in console", TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKConsole"));
  return test.result();
}
