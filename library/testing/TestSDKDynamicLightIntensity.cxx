#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

int TestSDKDynamicLightIntensity([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& Scene = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  Scene.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  // Check render with default light intensity
  test("render with default light intensity", TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDynamicLightIntensity-default"));

  // set light intensity to 5x brighter
  opt.render.light.intensity = 5.;
  test("render with light intensity of 5", TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDynamicLightIntensity-5x-brighter"));

  // set light intensity to 5x darker
  opt.render.light.intensity = .2;
  test("render with light intensity of .2", TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDynamicLightIntensity-5x-darker"));

  return test.result();
}
