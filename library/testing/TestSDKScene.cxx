#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

namespace fs = std::filesystem;

int TestSDKScene([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow().setSize(300, 300);

  // Test file logic
  std::string empty;
  std::string dummyFilename = "dummy.foo";
  std::string nonExistentFilename = "nonExistent.vtp";
  std::string unsupportedFilename = "unsupportedFile.dummy";
  std::string logoFilename = "mb/recursive/f3d.glb";
  std::string sphere1Filename = "mb/recursive/mb_1_0.vtp";
  std::string sphere2Filename = "mb/recursive/mb_2_0.vtp";
  std::string cubeFilename = "mb/recursive/mb_0_0.vtu";
  std::string worldFilename = "world.obj";
  std::string dummy = std::string(argv[1]) + "data/" + dummyFilename;
  std::string nonExistent = std::string(argv[1]) + "data/" + nonExistentFilename;
  std::string unsupported = std::string(argv[1]) + "data/" + unsupportedFilename;
  std::string logo = std::string(argv[1]) + "data/" + logoFilename;
  std::string sphere1 = std::string(argv[1]) + "data/" + sphere1Filename;
  std::string sphere2 = std::string(argv[1]) + "data/" + sphere2Filename;
  std::string cube = std::string(argv[1]) + "data/" + cubeFilename;
  std::string world = std::string(argv[1]) + "data/" + worldFilename;

  // supports method
  test("supported with empty filename", !sce.supports(empty));
  test("supported with dummy filename", !sce.supports(dummy));
  test("supported with non existent filename", sce.supports(nonExistent));
  test("supported with default scene format", sce.supports(cube));
  test("supported with full scene format", sce.supports(logo));

  // add error code paths
  test.expect<f3d::scene::load_failure_exception>("add with dummy file", [&]() { sce.add(dummy); });
  test.expect<f3d::scene::load_failure_exception>(
    "add with unsupported file", [&]() { sce.add(unsupported); });
  test.expect<f3d::scene::load_failure_exception>(
    "add with inexistent file", [&]() { sce.add(nonExistent); });

  // add standard code paths
  test("add with empty file", [&]() { sce.add(std::vector<std::string>{}); });
  test("add with empty file", [&]() { sce.add(empty); });
  test("add with a single path", [&]() { sce.add(fs::path(logo)); });
  test("add with multiples filepaths", [&]() { sce.add({ fs::path(sphere2), fs::path(cube) }); });
  test("add with multiples file strings", [&]() { sce.add({ sphere1, world }); });

  // render test
  test("render after add", [&]() {
    if (!TestSDKHelpers::RenderTest(
          win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKScene"))
    {
      throw "rendering test failed";
    }
  });

  // light test
  f3d::light_state_t defaultLight;
  f3d::light_state_t redLight = defaultLight;
  redLight.color = f3d::color_t(1.0, 0.0, 0.0);
  test("empty light count", [&]() {
    sce.removeAllLights();
    return sce.getLightCount() == 0;
  });
  test("add default light", [&]() {
    int index = sce.addLight(defaultLight);
    return index == 0 && sce.getLightCount() == 1;
  });
  test("add red light", [&]() {
    int index = sce.addLight(redLight);
    return index == 1 && sce.getLightCount() == 2;
  });
  test("light count after add", [&]() { return sce.getLightCount() == 2; });
  test("get light at index 0", [&]() {
    f3d::light_state_t light = sce.getLight(0);
    return light == defaultLight;
  });
  test("get light at index 1", [&]() {
    f3d::light_state_t light = sce.getLight(1);
    return light == redLight;
  });
  test.expect<f3d::scene::light_exception>(
    "get light at invalid index", [&]() { std::ignore = sce.getLight(10); });
  test.expect<f3d::scene::light_exception>(
    "update light at invalid index", [&]() { sce.updateLight(10, redLight); });
  sce.updateLight(0, redLight);
  test("update light", sce.getLight(0) == sce.getLight(1));
  test.expect<f3d::scene::light_exception>(
    "remove light at invalid index", [&]() { sce.removeLight(10); });
  test("remove light at index 0", [&]() {
    sce.removeLight(0);
    return sce.getLightCount() == 1;
  });

  test("render after light", [&]() {
    if (!TestSDKHelpers::RenderTest(
          win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneRedLight"))
    {
      throw "rendering test failed";
    }
  });

  return test.result();
}
