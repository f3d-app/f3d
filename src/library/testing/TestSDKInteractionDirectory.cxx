#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_options.h>
#include <f3d_window.h>

int TestSDKInteractionDirectory(int argc, char* argv[])
{
  // This test mimics TestInteractionDirectory.
  f3d::engine eng(f3d::engine::WindowTypeEnum::WINDOW_STANDARD);
  f3d::options& options = eng.getOptions();
  options.set("resolution", { 300, 300 });
  options.set("scalars", "");
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/mb");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();

  std::string filename = "TestSDKInteractionDirectory";
  inter.playInteraction(std::string(argv[1]) + "recordings/" + filename + ".log");
  return win.renderAndCompareWithFile(std::string(argv[1]) + "/baselines/" + filename + ".png", 50,
           false, std::string(argv[2]) + filename + ".png")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
