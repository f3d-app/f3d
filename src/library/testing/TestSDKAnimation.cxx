#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_window.h>

int TestSDKAnimation(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::WindowTypeEnum::WINDOW_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/InterpolationTest.glb");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();

  inter.startAnimation();
  inter.toggleAnimation();
  if (inter.isPlayingAnimation())
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
