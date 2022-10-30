#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDynamicLightIntensity(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  load.addFile(std::string(argv[1]) + "/data/cow.vtp").loadFile();

  win.render();

  // Check render with default light intensity
  if (!TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
        std::string(argv[2]), "TestSDKDynamicLightIntensity-default", 50))
  {
    std::cerr << "failed for default light intensity" << std::endl;
    return EXIT_FAILURE;
  }

  // set light-intensity to 5x brighter
  opt.set("render.light.intensity", 5.);
  if (!TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
        std::string(argv[2]), "TestSDKDynamicLightIntensity-5x-brighter", 50))
  {
    std::cerr << "failed for light intensity = 5" << std::endl;
    return EXIT_FAILURE;
  }

  // set light-intensity to 5x darker
  opt.set("render.light.intensity", .2);
  if (!TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
        std::string(argv[2]), "TestSDKDynamicLightIntensity-5x-darker", 50))
  {
    std::cerr << "failed for light intensity = .2" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
