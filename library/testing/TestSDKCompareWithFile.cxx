#include <engine.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKCompareWithFile(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKCompareWithFile")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
