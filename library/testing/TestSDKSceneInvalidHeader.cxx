#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>

int TestSDKSceneInvalidHeader([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();

  // Test file logic
  // To be merged in TestSDKScene after VTK v9.6 support is dropped
  std::string invalidHeaderFilename = "invalid.mdl";
  std::string invalidHeader = std::string(argv[1]) + "data/" + invalidHeaderFilename;

  // supports method
  test("not supported with invalid header", !sce.supports(invalidHeader));

  return test.result();
}
