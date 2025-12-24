#include <engine.h>
#include <interactor.h>
#include <scene.h>

int TestSDKStartInteractor([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  // Using an onscreen window to mimic standard usage
  f3d::engine eng = f3d::engine::create();
  f3d::scene& sce = eng.getScene();
  sce.add(std::string(argv[1]) + "/data/cow.vtp");
  f3d::interactor& inter = eng.getInteractor();
  inter.start(0.1, [&inter]() { inter.stop(); });

  // Call start multiple times
  inter.start(0.1, [&inter]() {
    inter.start(1, []() {});
    inter.stop();
  });

  // Call stop without loop running
  inter.stop();

  // Event loop with non positive deltaTime
  inter.triggerEventLoop(0);
  return EXIT_SUCCESS;
}
