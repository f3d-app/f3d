#include <engine.h>
#include <export.h>
#include <loader.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <random>

int TestSDKDynamicHDRI(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO);

  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);

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

  // Generate a random cache path to avoid reusing any existing cache
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> dist(1, 100000);
  std::string cachePath = std::string(argv[2]) + "/cache_" + std::to_string(dist(e1));
  eng.setCachePath(cachePath);

  // Change the hdri and make sure it is taken into account
  opt.set("render.hdri.file", std::string(argv[1]) + "data/palermo_park_1k.hdr");
  opt.set("render.hdri.ambient", true);
  opt.set("render.background.skybox", true);

  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRI", 50);
  if (!ret)
  {
    std::cerr << "Render with HDRI failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Check caching is working
  std::ifstream lutFile(cachePath + "/lut.vti");
  if (!lutFile.is_open())
  {
    std::cerr << "LUT cache file not found" << std::endl;
    return EXIT_FAILURE;
  }

  // Force a cache path change to force a LUT reconfiguration and test dynamic cache path
  eng.setCachePath(std::string(argv[2]) + "/cache_" + std::to_string(dist(e1)));
  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRI", 50);
  if (!ret)
  {
    std::cerr << "Render with HDRI with another cache path failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Use an existing cache
  eng.setCachePath(cachePath);
  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRI", 50);
  if (!ret)
  {
    std::cerr << "Render with HDRI with existing cache path failed" << std::endl;
    return EXIT_FAILURE;
  }

#if F3D_MODULE_EXR
  // Change the hdri and make sure it is taken into account
  opt.set("render.hdri.file", std::string(argv[1]) + "/data/kloofendal_43d_clear_1k.exr");
  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRIExr", 50);

  if (!ret)
  {
    std::cerr << "Render with EXR HDRI failed" << std::endl;
    return EXIT_FAILURE;
  }
#endif

#ifndef F3D_NO_DEPRECATED
  // Check deprecated HDRI options
  opt.set("render.hdri.ambient", false);
  opt.set("render.background.skybox", false);
  opt.set("render.background.hdri", std::string(argv[1]) + "data/palermo_park_1k.hdr");

  ret = TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
    std::string(argv[2]), "TestSDKDynamicHDRI", 50);
  if (!ret)
  {
    std::cerr << "Render with deprecated HDRI option failed" << std::endl;
    return EXIT_FAILURE;
  }
#endif

  return EXIT_SUCCESS;
}
