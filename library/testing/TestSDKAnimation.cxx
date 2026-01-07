#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>

int TestSDKAnimation([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();
  sce.add(std::string(argv[1]) + "/data/f3d.glb");

  test("availableAnimations", sce.availableAnimations() == 1);

  sce.loadAnimationTime(0.5);
  test("recover animationTimeRange", sce.animationTimeRange() == std::make_pair(0.0, 4.0));

  inter.startAnimation();
  test("isPlaying after start", inter.isPlayingAnimation());

  inter.toggleAnimation();
  test("isPlaying after toggle off", !inter.isPlayingAnimation());

  inter.toggleAnimation();
  test("isPlaying after toggle on", inter.isPlayingAnimation());

  f3d::interactor& interRef = inter.triggerEventLoop(0.1);
  test("triggerEventLoop returns self", &interRef == &inter);

  inter.stopAnimation();
  test("isPlaying after stop", !inter.isPlayingAnimation());

  test("getAnimationName returns name at index", inter.getAnimationName(0), "unnamed_0");

  test("getAnimationName returns for out of range", inter.getAnimationName(9999), "No animation");

  test("getAnimationName returns current name", inter.getAnimationName(), "unnamed_0");

  test("getAnimationNames returns names", inter.getAnimationNames(), std::vector<std::string>{"unnamed_0"});

  return test.result();
}
