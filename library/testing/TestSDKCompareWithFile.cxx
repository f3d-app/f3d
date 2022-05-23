#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

int TestSDKCompareWithFile(int argc, char* argv[])
{
  f3d::engine eng(
    f3d::engine::CREATE_WINDOW | f3d::engine::CREATE_INTERACTOR | f3d::engine::WINDOW_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 });
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();
  f3d::image result = win.renderToImage();
  f3d::image diff;
  bool ret = result.compare(
    f3d::image(std::string(argv[1]) + "/baselines/TestSDKCompareWithFile.png"), diff, 50);
  if (!ret)
  {
    result.save(std::string(argv[2]) + "TestSDKCompareWithFile.png");
    diff.save(std::string(argv[2]) + "TestSDKCompareWithFile.diff.png");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
