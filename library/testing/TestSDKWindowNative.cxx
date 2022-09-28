#include <engine.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKWindowNative(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng;
  f3d::window& win = eng.getWindow();
  win.setWindowName("Test").setSize(300, 300).setPosition(100, 100);

  if (win.getWidth() != 300 || win.getHeight() != 300)
  {
    std::cerr << "Unexpected window size: " << win.getWidth() << "x" << win.getHeight()
              << std::endl;
    return EXIT_FAILURE;
  }

  if (win.getType() != f3d::window::Type::NATIVE)
  {
    std::cerr << "Unexpected window type" << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options& options = eng.getOptions();
  options.set("render.background.color", { 0.8, 0.2, 0.9 });

  // Use a higher threshold as background difference can be strong with mesa
  return TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
           "TestSDKWindowStandard", 150)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
