#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <window.h>

int TestSDKWindowNoCache([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  eng.setCachePath(std::string(argv[1]) + "/data/corrupted_cache");
  f3d::window& win = eng.getWindow();
  win.setSize(300, 300);

  eng.getScene().add(std::string(argv[1]) + "/data/suzanne.ply");

  eng.getOptions().model.material.roughness = 0.0;
  eng.getOptions().render.hdri.file = std::string(argv[1]) + "/data/shanghai_bund_1k.hdr";
  eng.getOptions().render.hdri.ambient = true;

  // render with the corrupted cache (redish spherical harmonics and low-res LUT/specular)
  test("render with corrupted cache",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
      "TestSDKWindowCacheCorrupted"));

  win.setUseHDRICache(false);

  // render with the cache disabled
  test("render with no cache",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
      "TestSDKWindowNoCache"));

  return test.result();
}
