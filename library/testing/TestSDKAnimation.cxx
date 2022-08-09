#include <engine.h>
#include <interactor.h>
#include <loader.h>

#include <iostream>

int TestSDKAnimation(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::interactor& inter = eng.getInteractor();
  load.addFile(std::string(argv[1]) + "/data/InterpolationTest.glb").loadFile();

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
