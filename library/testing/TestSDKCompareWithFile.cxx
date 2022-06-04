#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKCompareWithFile(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 });
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKCompareWithFile", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
