#include <engine.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKConsole(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO);

  // test warning with no color
  f3d::log::setUseColoring(false);
  f3d::log::warn("Message in console");

  f3d::engine eng = f3d::engine::create(true);

  f3d::options& opt = eng.getOptions();
  opt.ui.console = true;

  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  return TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKConsole")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
