#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

int TestSDKCompareWithFile([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  test("Compare render with baseline",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKCompareWithFile"));
  return test.result();
}
