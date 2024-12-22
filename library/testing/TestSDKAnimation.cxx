#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>

int TestSDKAnimation(int argc, char* argv[])
{
  PseudoUnitTest test;
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();
  sce.add(std::string(argv[1]) + "/data/f3d.glb");

  sce.loadAnimationTime(0.5);
  test("recover animationTimeRange", sce.animationTimeRange() == std::make_pair(0.0, 4.0));

  inter.startAnimation();
  test("isPlaying after start", inter.isPlayingAnimation());

  inter.toggleAnimation();
  test("isPlaying after toggle off", !inter.isPlayingAnimation());

  inter.toggleAnimation();
  test("isPlaying after toggle on", inter.isPlayingAnimation());

  inter.stopAnimation();
  test("isPlaying after stop", !inter.isPlayingAnimation());

  return test.result();
}
