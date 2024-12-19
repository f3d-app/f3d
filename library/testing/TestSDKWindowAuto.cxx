#include <engine.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKWindowAuto(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  win.setWindowName("Test").setSize(300, 300).setPosition(100, 100);

  if (win.getWidth() != 300 || win.getHeight() != 300)
  {
    std::cerr << "Unexpected window size: " << win.getWidth() << "x" << win.getHeight()
              << std::endl;
    return EXIT_FAILURE;
  }

  if (win.getType() == f3d::window::Type::UNKNOWN)
  {
    std::cerr << "Unexpected window type" << std::endl;
    return EXIT_FAILURE;
  }

  if (!win.isOffscreen())
  {
    std::cerr << "Window should be offscreen" << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options& options = eng.getOptions();
  options.render.background.color = { 0.8, 0.2, 0.9 };

  // XXX: Use a higher threshold as background difference can be strong with older versions of VTK
  // This can be removed once VTK 9.3 support is removed
  return TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
           "TestSDKWindowStandard", 0.12)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
