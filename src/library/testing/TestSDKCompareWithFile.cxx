#include <f3d_engine.h>
#include <f3d_loader.h>
#include <f3d_options.h>
#include <f3d_window.h>

int TestSDKCompareWithFile(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::WINDOW_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 });
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();
  bool ret =
    win.renderAndCompareWithFile(std::string(argv[1]) + "/baselines/TestSDKCompareWithFile.png", 50,
      false, std::string(argv[2]) + "TestSDKCompareWithFile.png");
  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
