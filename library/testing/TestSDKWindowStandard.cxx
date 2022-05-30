#include <engine.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKWindowStandard(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::CREATE_WINDOW);
  f3d::window& win = eng.getWindow();
  win.setWindowName("Test");

  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 })
    .set("background-color", { 0.8, 0.2, 0.9 })
    .set("verbose", true);
  win.update();

  // Use a higher threshold as background difference can be strong with mesa
  return TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
           "TestSDKWindowStandard", 150)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
