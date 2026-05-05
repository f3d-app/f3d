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
  inter.setEventLoopUserCallBack([&inter](f3d::interactor_state_t) { inter.stop(); });
  inter.start(0.1);

  // Call start multiple times
  inter.setEventLoopUserCallBack([&inter](f3d::interactor_state_t) {
    // Trying to set a callback when loop is already running. Should not do anything.
    inter.setEventLoopUserCallBack([&inter](f3d::interactor_state_t) {});

    inter.start();
    inter.stop();
  });
  inter.start(0.1);

  // Call stop without loop running
  inter.stop();

  // Event loop with non positive deltaTime
  inter.triggerEventLoop(0);
  return EXIT_SUCCESS;
}
