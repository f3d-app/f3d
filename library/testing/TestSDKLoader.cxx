#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoader(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NONE);
  f3d::loader& load = eng.getLoader();

  // Test file logic
  std::string empty;
  std::string dummyFilename = "dummy.foo";
  std::string nonExistentGeometryFilename = "nonExistent.vtp";
  std::string nonExistentFullSceneFilename = "nonExistent.obj";
  std::string invalidGeometryFilename = "invalid.vtp";
  std::string invalidFullSceneFilename = "invalid.gltf";
  std::string unsupportedFilename = "unsupportedFile.dummy";
  std::string cowFilename = "cow.vtp";
  std::string dragonFilename = "dragon.vtu";
  std::string suzanneFilename = "suzanne.stl";
  std::string worldFilename = "world.obj";
  std::string botFilename = "bot2.wrl";
  std::string dummy = std::string(argv[1]) + "data/" + dummyFilename;
  std::string nonExistentGeometry = std::string(argv[1]) + "data/" + nonExistentGeometryFilename;
  std::string nonExistentFullScene = std::string(argv[1]) + "data/" + nonExistentFullSceneFilename;
  std::string invalidGeometry = std::string(argv[1]) + "data/" + invalidGeometryFilename;
  std::string invalidFullScene = std::string(argv[1]) + "data/" + invalidFullSceneFilename;
  std::string unsupported = std::string(argv[1]) + "data/" + unsupportedFilename;
  std::string cow = std::string(argv[1]) + "data/" + cowFilename;
  std::string dragon = std::string(argv[1]) + "data/" + dragonFilename;
  std::string suzanne = std::string(argv[1]) + "data/" + suzanneFilename;
  std::string world = std::string(argv[1]) + "data/" + worldFilename;
  std::string bot = std::string(argv[1]) + "data/" + botFilename;

  // has*Reader methods
  if (load.hasGeometryReader(empty) || load.hasSceneReader(empty))
  {
    std::cerr << "Unexpected has*Reader output with empty filenames" << std::endl;
    return EXIT_FAILURE;
  }
  if (load.hasGeometryReader(dummy) || load.hasSceneReader(dummy))
  {
    std::cerr << "Unexpected has*Reader output with dummy filenames" << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.hasGeometryReader(nonExistentGeometry) || !load.hasSceneReader(nonExistentFullScene))
  {
    std::cerr << "Unexpected has*Reader output with non existent filenames" << std::endl;
    return EXIT_FAILURE;
  }
  if (load.hasGeometryReader(bot) || load.hasSceneReader(dragon))
  {
    std::cerr << "Unexpected has*Reader output with incorrect formats" << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.hasGeometryReader(dragon) || !load.hasSceneReader(bot))
  {
    std::cerr << "Unexpected has*Reader output with correct formats" << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.hasGeometryReader(world) || !load.hasSceneReader(world))
  {
    std::cerr << "Unexpected has*Reader output with geometry and full scene format" << std::endl;
    return EXIT_FAILURE;
  }

  // Empty filename, success expected but nothing is loaded
  try
  {
    load.loadGeometry(empty);
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadGeometry failure with an empty file" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    load.loadFullScene(empty);
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadGeometry failure with an empty file" << std::endl;
    return EXIT_FAILURE;
  }

  // Dummy filename
  try
  {
    load.loadGeometry(dummy);
    std::cerr << "Unexpected loadGeometry success with a dummy file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  try
  {
    load.loadFullScene(dummy);
    std::cerr << "Unexpected loadGeometry success with a dummy file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Non supported files
  try
  {
    load.loadGeometry(unsupported);
    std::cerr << "Unexpected loadGeometry success with an unsupported file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  try
  {
    load.loadFullScene(unsupported);
    std::cerr << "Unexpected loadFullScene success with an unsupported file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Incorrect files
  try
  {
    load.loadGeometry(bot);
    std::cerr << "Unexpected loadGeometry success with an incorrect file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  try
  {
    load.loadFullScene(cow);
    std::cerr << "Unexpected loadFullScene success with an incorrect file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Non existent files
  try
  {
    load.loadGeometry(nonExistentGeometry);
    std::cerr << "Unexpected loadGeometry success with a non existent file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  try
  {
    load.loadFullScene(nonExistentFullScene);
    std::cerr << "Unexpected loadFullScene success with a non existent file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Invalid files XXX should NOT succeed but not supported by vtkImporter yet
  try
  {
    load.loadGeometry(invalidGeometry);
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadGeometry failure with an invalid file" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    load.loadFullScene(invalidFullScene);
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadFullScene failure with an invalid file" << std::endl;
    return EXIT_FAILURE;
  }

  // Multiple geometries
  try
  {
    load.loadGeometry(cow).loadGeometry(suzanne).loadGeometry(dragon);
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadGeometry failure with multiple files" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
