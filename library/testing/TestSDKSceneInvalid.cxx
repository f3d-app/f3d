#include "PseudoUnitTest.h"

#include <engine.h>
#include <log.h>
#include <scene.h>

int TestSDKSceneInvalid(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::createNone();
  f3d::scene& sce = eng.getScene();

  std::string validFilename = "cow.vtp";
  std::string invalidDefaultSceneFilename = "invalid.vtp";
  std::string invalidFullSceneFilename = "duck_invalid.gltf";
  std::string validePath = std::string(argv[1]) + "data/" + validFilename;
  std::string invalidDefaultScene = std::string(argv[1]) + "data/" + invalidDefaultSceneFilename;
  std::string invalidFullScene = std::string(argv[1]) + "data/" + invalidFullSceneFilename;

  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid default scene file", [&]() { sce.add(invalidDefaultScene); });
  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid full scene file", [&]() { sce.add(invalidFullScene); });
  test.expect<f3d::scene::load_failure_exception>("add with invalid multiple files",
    [&]() { sce.add({ validFilename, invalidFullScene, invalidDefaultScene }); });

  return test.result();
}
