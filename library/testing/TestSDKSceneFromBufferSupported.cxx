#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <scene.h>

#include <fstream>
#include <vector>

int TestSDKSceneFromBufferSupported([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  const std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();

  opt.scene.force_reader = "PLYReader";
  const std::string validFilePath = std::string(argv[1]) + "data/points.ply";
  std::ifstream validFile(validFilePath, std::ios::binary);
  std::vector<char> validBuffer(
    (std::istreambuf_iterator<char>(validFile)), std::istreambuf_iterator<char>());
  test("supported with a valid buffer",
    sce.supports(reinterpret_cast<const std::byte*>(validBuffer.data()), validBuffer.size()) ==
      f3d::file_availability::SUPPORTED);

  return test.result();
}
