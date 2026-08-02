#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>

int TestSDKSceneInvalidHeader([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();

  // Test file logic
  // To be merged in TestSDKScene after VTK v9.6 support is dropped
  std::string invalidHeaderFilename = "invalid.mdl";
  std::string invalidHeader = std::string(argv[1]) + "data/" + invalidHeaderFilename;

  // supports method
  test("not supported with invalid header",
    sce.supports(invalidHeader) == f3d::file_availability::UNSUPPORTED_CONTENT);
  eng.getOptions().scene.skip_content_check = true;
  test("skip content check", sce.supports(invalidHeader) == f3d::file_availability::SUPPORTED);

  // add method with content check on (default)
  {
    f3d::engine engine = f3d::engine::create(true);
    f3d::scene& scene = engine.getScene();
    test.expect<f3d::scene::load_failure_exception>(
      "add with invalid header, exception type check", [&]() { scene.add(invalidHeader); });
    try
    {
      scene.add(invalidHeader);
    }
    catch (f3d::scene::load_failure_exception& E)
    {
      std::string expectedMsg =
        "contains unsupported content and no reader have been selected, use skip content check to "
        "skip content validation or force reader to force a specific reader";
      std::string exceptMsg = E.what();
      test("Check exception message size", exceptMsg.size() >= expectedMsg.size());
      test("Check exception message",
        exceptMsg.substr(exceptMsg.size() - expectedMsg.size(), expectedMsg.size()) == expectedMsg);
    }
  }

  return test.result();
}
