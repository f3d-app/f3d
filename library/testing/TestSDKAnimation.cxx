#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>

using namespace std::string_literals;

int TestSDKAnimation([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;
  const std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::animation& anim = sce.getAnimation();
  f3d::interactor& inter = eng.getInteractor();

  test("animations count for empty scene", anim.count() == 0);

  test("getName returns for empty scene", anim.getName(), "No animation"s);

  test("getNames returns 0 len vec for empty scene", anim.getNames().size() == 0);

  sce.add(std::string(argv[1]) + "/data/soldier_animations.mdl");

  test("animations count", anim.count() == 10);

  anim.loadTime(0.5);
  test("recover timeRange", anim.getTimeRange() == std::make_pair(0.0, 0.7999999999999999));

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

  test("getName returns name at index", anim.getName(0), "stand"s);

  test("getName returns for out of range", anim.getName(9999), "No animation"s);

  test("getName returns current name", anim.getName(), "stand"s);

  test("getNames returns names", anim.getNames(),
    std::vector<std::string>{
      "stand", "dead", "dead_right", "reload", "hit", "down", "stumble", "run", "shoot", "walk" });

  auto keyframes = anim.getKeyFrames();
  test("check keyframes size", static_cast<int>(keyframes.size()), 9);
  test("check first keyframes", keyframes[0], 0.0);
  test("check last keyframes", keyframes[8], 0.7999999999999999);

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
