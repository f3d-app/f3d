#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

namespace fs = std::filesystem;

int TestSDKScene(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::ERROR);
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

  return test.result();
}
