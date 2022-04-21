#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_window.h>

#include <iostream>

int TestSDKAnimation(int argc, char* argv[])
{
  f3d::engine eng(
    f3d::engine::CREATE_WINDOW | f3d::engine::CREATE_INTERACTOR | f3d::engine::WINDOW_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  load.addFile(std::string(argv[1]) + "/data/InterpolationTest.glb");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  inter.startAnimation();
  if (!inter.isPlayingAnimation())
  {
    std::cerr << "Animation is not playing after being started." << std::endl;
    return EXIT_FAILURE;
  }

  inter.toggleAnimation();
  if (inter.isPlayingAnimation())
  {
    std::cerr << "Animation is playing after being toggled off." << std::endl;
    return EXIT_FAILURE;
  }

  inter.toggleAnimation();
  if (!inter.isPlayingAnimation())
  {
    std::cerr << "Animation is not playing after being toggled on." << std::endl;
    return EXIT_FAILURE;
  }

  inter.stopAnimation();
  if (inter.isPlayingAnimation())
  {
    std::cerr << "Animation is playing after being stopped." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
