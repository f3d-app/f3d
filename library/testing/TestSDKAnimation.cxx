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

  test("availableAnimations for empty scene", sce.availableAnimations() == 0);

  test("getAnimationName returns for empty scene", sce.getAnimationName(),
    std::string("No animation"));

  test("getAnimationNames returns 0 len vec for empty scene", sce.getAnimationNames().size() == 0);

  sce.add(std::string(argv[1]) + "/data/f3d.glb");

  test("availableAnimations", sce.availableAnimations() == 1);

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

  test("getAnimationName returns name at index", sce.getAnimationName(0), std::string("unnamed_0"));

  test("getAnimationName returns for out of range", sce.getAnimationName(9999),
    std::string("No animation"));

  test("getAnimationName returns current name", sce.getAnimationName(), std::string("unnamed_0"));

  test("getAnimationNames returns names", sce.getAnimationNames(),
    std::vector<std::string>{ "unnamed_0" });
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
