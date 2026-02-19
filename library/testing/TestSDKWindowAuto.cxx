#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <window.h>

int TestSDKWindowAuto([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  win.setWindowName("Test").setSize(300, 300).setPosition(100, 100);

  test("window width", win.getWidth(), 300);
  test("window height", win.getHeight(), 300);
  test("window type", win.getType() != f3d::window::Type::UNKNOWN);
  test("window offscreen", win.isOffscreen());

  f3d::options& options = eng.getOptions();
  options.render.background.color = { 0.8, 0.2, 0.9 };

  // XXX: Use a higher threshold as background difference can be strong with older versions of VTK
  // This can be removed once VTK 9.3 support is removed
  test("render with auto window",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
      "TestSDKWindowStandard", 0.12));

  return test.result();
}
