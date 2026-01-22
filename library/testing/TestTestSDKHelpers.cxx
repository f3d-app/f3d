#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <export.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

int TestTestSDKHelpers([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& scene = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  scene.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  // Sanity checks
  test("render with empty baseline path",
    !TestSDKHelpers::RenderTest(
      eng.getWindow(), "", std::string(argv[2]), "TestTestSDKHelpersEmptyBaselinePath"));
  test("render with empty output path",
    !TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", "",
      "TestTestSDKHelpersEmptyOutputPath"));
  test("render with empty name",
    !TestSDKHelpers::RenderTest(
      eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), ""));

  // Inexistent baseline
  test("render with inexistent name",
    !TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "inexistent/",
      std::string(argv[2]), "TestTestSDKHelpersInexistentPath"));
  test("render with created baseline in temporary dir",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[2]), std::string(argv[2]),
      "TestTestSDKHelpersInexistentPath"));

  // Error with low threshold
  test("render with low threshold",
    !TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestTestSDKHelpersThreshold", 0.00001));
  test("render with created baseline with low threshold",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[2]), std::string(argv[2]),
      "TestTestSDKHelpersThreshold", 0.00001));

  //  Degrees2Radians(double degrees)
  test("degrees to radians", TestSDKHelpers::Degrees2Radians(45), .78539816339744830961);

  return test.result();
}
