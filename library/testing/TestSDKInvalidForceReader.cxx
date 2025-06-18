#include "PseudoUnitTest.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

namespace fs = std::filesystem;
int TestSDKInvalidForceReader(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine::autoloadPlugins();

  // Test file path setup
  std::string monkey = std::string(argv[1]) + "data/red_translucent_monkey.gltf";

  // Test INVALID reader
  {
    f3d::engine engine = f3d::engine::create(true);
    engine.getOptions().scene.force_reader = "INVALID";
    f3d::scene& scene = engine.getScene();
    test.expect<f3d::scene::load_failure_exception>(
      "Handling wrong force reader, exception type check", [&]() { scene.add(fs::path(monkey)); });
    try
    {
      scene.add(fs::path(monkey));
    }
    catch (f3d::scene::load_failure_exception& E)
    {
      std::string expectedMsg = "is not a valid force reader";
      std::string exceptMsg = E.what();
      test("Check exception message size", exceptMsg.size() >= expectedMsg.size());
      test("Check exception message",
        exceptMsg.substr(exceptMsg.size() - expectedMsg.size(), expectedMsg.size()) == expectedMsg);
    }
  }

  return test.result();
}
