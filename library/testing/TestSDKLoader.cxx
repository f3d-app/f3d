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

  std::string filePath, fileName, fileInfo;

  // Test file logic
  std::string dummyFilename = "dummy.foo";
  std::string cowFilename = "cow.vtp";
  std::string dragonFilename = "dragon.vtu";
  std::string suzanneFilename = "suzanne.stl";
  std::string worldFilename = "world.obj";
  std::string botFilename = "bot2.wrl";
  std::string dummy = std::string(argv[1]) + "data/" + dummyFilename;
  std::string cow = std::string(argv[1]) + "data/" + cowFilename;
  std::string dragon = std::string(argv[1]) + "data/" + dragonFilename;
  std::string suzanne = std::string(argv[1]) + "data/" + suzanneFilename;
  std::string world = std::string(argv[1]) + "data/" + worldFilename;
  std::string bot = std::string(argv[1]) + "data/" + botFilename;

  // canRead methods
  if (load.canReadGeometry(dummy) || load.canReadScene(dummy))
  {
    std::cerr << "Unexpected canRead output with dummy filenames" << std::endl;
    return EXIT_FAILURE;
  }
  if (load.canReadGeometry(bot) || load.canReadScene(dragon))
  {
    std::cerr << "Unexpected canRead output with incorrect formats" << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.canReadGeometry(dragon) || !load.canReadScene(bot))
  {
    std::cerr << "Unexpected canRead output with correct formats" << std::endl;
    return EXIT_FAILURE;
  }
  if (!load.canReadGeometry(world) || !load.canReadScene(world))
  {
    std::cerr << "Unexpected canRead output with geometry and full scene format" << std::endl;
    return EXIT_FAILURE;
  }

  // Geometry without default scene
  if (load.addGeometry(cow))
  {
    std::cerr << "Unexpected addGeometry success without a default scene" << std::endl;
    return EXIT_FAILURE;
  }

  // Non existent filename
  if (load.resetToDefaultScene().addGeometry(dummy))
  {
    std::cerr << "Unexpected addGeometry success with a dummy file" << std::endl;
    return EXIT_FAILURE;
  }

  if (load.loadFullScene(dummy))
  {
    std::cerr << "Unexpected loadFullScene success with a dummy file" << std::endl;
    return EXIT_FAILURE;
  }

  // Non supported files
  if (load.resetToDefaultScene().addGeometry(bot))
  {
    std::cerr << "Unexpected addGeometry success with an incorrect file" << std::endl;
    return EXIT_FAILURE;
  }

  if (load.loadFullScene(cow))
  {
    std::cerr << "Unexpected loadFullScene success with an incorrect file" << std::endl;
    return EXIT_FAILURE;
  }

  // Multiple geometries
  if (!load.resetToDefaultScene().addGeometry(cow))
  {
    std::cerr << "Unexpected addGeometry failure with first file" << std::endl;
    return EXIT_FAILURE;
  }

  if (!load.addGeometry(suzanne))
  {
    std::cerr << "Unexpected addGeometry failure with second file" << std::endl;
    return EXIT_FAILURE;
  }

  if (!load.addGeometry(dragon))
  {
    std::cerr << "Unexpected addGeometry failure with third file" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
