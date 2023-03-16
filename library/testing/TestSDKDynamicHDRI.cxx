#include <engine.h>
#include <loader.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <random>

int TestSDKDynamicHDRI(int argc, char* argv[])
{
  // Generate a random number to avoid reusing any existing cache
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> dist(1, 100000);
  std::string cachePath = std::string(argv[2]) + "/cache_" + std::to_string(dist(e1));

  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  eng.setCachePath(cachePath);

  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.set("ui.filename", true);
  opt.set("ui.filename-info", "(1/1) cow.vtp");

  load.loadGeometry(std::string(argv[1]) + "/data/cow.vtp");

  bool ret = win.render();

  if (!ret)
  {
    std::cerr << "First render failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Change the hdri and make sure it is taken into account
  opt.set("render.background.hdri", std::string(argv[1]) + "data/palermo_park_1k.hdr");

  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRI", 50);

  if (!ret)
  {
    std::cerr << "Second render with HDRI failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Check caching is working
  std::ifstream lutFile(cachePath + "/lut.vti");

  if (!lutFile.is_open())
  {
    std::cerr << "LUT cache file not found" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
