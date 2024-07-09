#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoaderInvalid(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NONE);
  f3d::loader& load = eng.getLoader();

  std::string invalidGeometryFilename = "invalid.vtp";
  std::string invalidFullSceneFilename = "duck_invalid.gltf";
  std::string invalidGeometry = std::string(argv[1]) + "data/" + invalidGeometryFilename;
  std::string invalidFullScene = std::string(argv[1]) + "data/" + invalidFullSceneFilename;

  try
  {
    load.loadGeometry(invalidGeometry);
    std::cerr << "Unexpected loadGeometry success with an invalid file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  try
  {
    load.loadScene(invalidFullScene);
    std::cerr << "Unexpected loadScene success with an invalid file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  return EXIT_SUCCESS;
}
