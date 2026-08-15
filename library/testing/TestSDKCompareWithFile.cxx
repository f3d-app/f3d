#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

int TestSDKCompareWithFile([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  test("Compare render with baseline",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKCompareWithFile"));
  return test.result();
}
