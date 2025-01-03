#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

namespace fs = std::filesystem;
int TestSDKReaderSelection(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine::autoloadPlugins();

  // Test file path setup
  std::string monkey = std::string(argv[1]) + "data/red_translucent_monkey.gltf";

  // Test default reader (no preference)
  {
    f3d::engine engine = f3d::engine::create(true);
    f3d::scene& scene = engine.getScene();
    test("add with a single path", [&]() { scene.add(fs::path(monkey)); });
  }

  // Test Draco reader
  {
    f3d::engine engine = f3d::engine::create(true);
    engine.getOptions().render.reader = "GLTFDraco";
    f3d::scene& scene = engine.getScene();
    test("Draco reader works", [&]() { scene.add(fs::path(monkey)); });
    test("Reader is GLTFDraco", engine.getReader() == "GLTFDraco");
  }

  // Test GLTF reader
  {
    f3d::engine engine = f3d::engine::create(true);
    engine.getOptions().render.reader = "GLTF";
    f3d::scene& scene = engine.getScene();
    test("GLTF reader works", [&]() { scene.add(fs::path(monkey)); });
    test("Reader is GLTF", engine.getReader() == "GLTF");
  }

  return test.result();
}