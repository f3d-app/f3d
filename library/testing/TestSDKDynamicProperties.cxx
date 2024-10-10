#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicProperties(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.filename = true;
  opt.ui.filename_info = "(1/1) cow.vtp";

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  // Change model properties and make sure it is taken into account
  opt.model.material.roughness = 0.6;
  opt.model.material.metallic = 0.5;
  opt.model.color.rgb = { 0.6, 0.1, 0.2 };
  opt.model.color.opacity = 0.6;

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDynamicProperties", 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
