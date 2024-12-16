#include <engine.h>
#include <interactor.h>
#include <scene.h>

int TestSDKStartInteractor(int argc, char* argv[])
{
  // Using an onscreen window to mimic standard usage
  f3d::engine eng = f3d::engine::create();
  f3d::scene& sce = eng.getScene();
  sce.add(std::string(argv[1]) + "/data/cow.vtp");
  f3d::interactor& inter = eng.getInteractor();
  inter.start(1, [&inter]() { inter.stop(); });
  return EXIT_SUCCESS;
}
