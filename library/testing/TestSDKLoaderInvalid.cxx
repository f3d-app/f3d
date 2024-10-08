#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoaderInvalid(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NONE);
  f3d::loader& load = eng.getLoader();

  std::string invalidDefaultSceneFilename = "invalid.vtp";
  std::string invalidFullSceneFilename = "duck_invalid.gltf";
  std::string invalidDefaultScene = std::string(argv[1]) + "data/" + invalidDefaultSceneFilename;
  std::string invalidFullScene = std::string(argv[1]) + "data/" + invalidFullSceneFilename;

  test.expect<f3d::loader::load_failure_exception>(
    "add with invalid default scene file", [&]() { load.add(invalidDefaultScene); });
  test.expect<f3d::loader::load_failure_exception>(
    "add with invalid full scene file", [&]() { load.add(invalidFullScene); });

  return test.result();
}
