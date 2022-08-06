#include <engine.h>
#include <loader.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKCompareWithFile(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  load.addFile(std::string(argv[1]) + "/data/cow.vtp").loadFile();

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKCompareWithFile", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
