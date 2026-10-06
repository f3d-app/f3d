#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <scene.h>

#include <fstream>
#include <vector>

int TestSDKSceneFromBuffer([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  const std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();

  // Add empty buffer
  test("Add empty buffer", [&]() { sce.add(nullptr, 0); });

  std::byte y{ 1 };

  // Add buffer without setting reader
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer without setting reader", [&]() { sce.add(&y, 1); });

  // Add buffer with invalid reader
  opt.scene.force_reader = "INVALID";
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer with invalid reader", [&]() { sce.add(&y, 1); });

  // Add buffer with reader that doesn't support streams
  opt.scene.force_reader = "Nrrd";
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer with reader that doesn't support streams", [&]() { sce.add(&y, 1); });

  opt.scene.force_reader = std::nullopt;

  // supports method
  test("not supported with null buffer",
    sce.supports(nullptr, 0) == f3d::file_availability::EMPTY_STREAM);
  test("not supported with zero size", sce.supports(&y, 0) == f3d::file_availability::EMPTY_STREAM);
  test("not supported with unrecognized buffer content",
    sce.supports(&y, 1) == f3d::file_availability::UNSUPPORTED_CONTENT);

  std::string validFilePath = std::string(argv[1]) + "data/cow.vtp";
  std::ifstream validFile(validFilePath, std::ios::binary);
  std::vector<char> validBuffer(
    (std::istreambuf_iterator<char>(validFile)), std::istreambuf_iterator<char>());
  test("supported with a valid buffer",
    sce.supports(reinterpret_cast<const std::byte*>(validBuffer.data()), validBuffer.size()) ==
      f3d::file_availability::SUPPORTED);

  return test.result();
}
