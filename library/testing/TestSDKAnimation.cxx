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

  sce.jumpToKeyFrame(1, true);
  sce.jumpToKeyFrame(-1, true);
  sce.jumpToKeyFrame(1, false);
  sce.jumpToKeyFrame(-1, false);

  sce.loadAnimationTime(0.5);
  test("recover animationTimeRange", sce.animationTimeRange() == std::make_pair(0.0, 4.0));

  inter.startAnimation();
  test("isPlaying after start", inter.isPlayingAnimation());
  test("isPlaying forward after start",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::FORWARD);

  inter.toggleAnimation();
  test("isPlaying after toggle off", !inter.isPlayingAnimation());

  inter.toggleAnimation();
  test("isPlaying after toggle on", inter.isPlayingAnimation());
  test("isPlaying forward toggle on",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::FORWARD);

  f3d::interactor& interRef = inter.triggerEventLoop(0.1);
  test("triggerEventLoop returns self", &interRef == &inter);

  inter.stopAnimation();
  test("isPlaying after stop", !inter.isPlayingAnimation());

  inter.startAnimation(f3d::interactor::AnimationDirection::FORWARD);
  test("isPlaying backward after forward start",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::FORWARD);
  inter.stopAnimation();

  inter.startAnimation(f3d::interactor::AnimationDirection::BACKWARD);
  test("isPlaying backward after backward start",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::BACKWARD);
  inter.stopAnimation();

  inter.toggleAnimation(f3d::interactor::AnimationDirection::FORWARD);
  test("isPlaying backward after forward toggle on",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::FORWARD);
  inter.stopAnimation();

  inter.toggleAnimation(f3d::interactor::AnimationDirection::BACKWARD);
  test("isPlaying backward after backward toggle on",
    inter.getAnimationDirection() == f3d::interactor::AnimationDirection::BACKWARD);
  inter.stopAnimation();

  return test.result();
}
