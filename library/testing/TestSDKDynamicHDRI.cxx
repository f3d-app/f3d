#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <export.h>
#include <log.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <random>

int TestSDKDynamicHDRI([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO);

  f3d::engine eng = f3d::engine::create(true);

  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.filename = true;
  opt.ui.filename_info = "(1/1) cow.vtp";

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  test("first render", [&]() { win.render(); });

  // Generate a random cache path to avoid reusing any existing cache
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> dist(1, 100000);
  std::string cachePath = std::string(argv[2]) + "/cache_" + std::to_string(dist(e1));
  eng.setCachePath(cachePath);

  // Enable HDRI ambient and skybox and check the default HDRI
  opt.render.hdri.ambient = true;
  opt.render.background.skybox = true;
  test("render with default HDRI",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicHDRIDefault"));

  // Change the hdri and make sure it is taken into account
  opt.render.hdri.file = std::string(argv[1]) + "data/palermo_park_1k.hdr";
  test("render with set HDRI",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicHDRI"));

  // Check caching is working
  std::ifstream lutFile(cachePath + "/lut.vti");
  test("open lut cache file", lutFile.is_open());

  // Force a cache path change to force a LUT reconfiguration and test dynamic cache path
  eng.setCachePath(std::string(argv[2]) + "/cache_" + std::to_string(dist(e1)));
  test("render with HDRI using another cache path",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicHDRI"));

  // Use an existing cache
  eng.setCachePath(cachePath);
  test("render with HDRI using existing cache path",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicHDRI"));

#if F3D_MODULE_EXR
  // Change the hdri and make sure it is taken into account
  opt.render.hdri.file = std::string(argv[1]) + "/data/kloofendal_43d_clear_1k.exr";
  test("render with EXR HDRI",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKDynamicHDRIExr"));
#endif

  return test.result();
}
