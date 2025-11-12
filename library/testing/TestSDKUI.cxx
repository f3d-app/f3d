#include <engine.h>
#include <options.h>
#include <scene.h>
#include <types.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKUI([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);

  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  f3d::options& options = eng.getOptions();

  // Set loader_progress_color only for coverage
  options.ui.loader_progress = true;
  options.ui.loader_progress_color = f3d::color_t(0.f, 1.f, 1.f);

  options.ui.animation_progress = true;
  options.ui.animation_progress_color = f3d::color_t(0.f, 1.f, 0.f);

  f3d::scene& sce = eng.getScene();
  sce.add(std::string(argv[1]) + "/data/BoxAnimated.gltf");
  sce.loadAnimationTime(0.5);

  win.render();

  if (!TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
        "TestSDKUIAnimationProgressColor"))
  {
    std::cerr << "Animation progress color failure";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
