#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <scene.h>

namespace
{
  struct streamHelper
  {
    streamHelper(const std::string& filepath)
      : Stream(filepath)
    {
      this->Stream.seekg(0, std::ios_base::end);
      this->Size = this->Stream.tellg();
      this->Stream.seekg(0, std::ios_base::beg);
      this->Stream.read(reinterpret_cast<char*>(this->Buffer.data()), this->Size);
    };

    std::ifstream Stream;
    std::vector<std::byte> Buffer;
    std::size_t Size;
  };
}

int TestSDKSceneFromBuffer([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();

  std::string empty;
  std::string unsupportedFilename = "unsupportedFile.dummy";
  std::string invalidHeaderFilename = "invalid.mdl";
  std::string invalidBodyFilename = "invalid_body.vtp";
  std::string logoFilename = "mb/recursive/f3d.glb";
  std::string sphere1Filename = "mb/recursive/mb_1_0.vtp";
  std::string sphere2Filename = "mb/recursive/mb_2_0.vtp";
  std::string cubeFilename = "mb/recursive/mb_0_0.vtu";
  std::string worldFilename = "world.obj";
  ::streamHelper unsupported(std::string(argv[1]) + "data/" + unsupportedFilename);
  ::streamHelper invalidBody(std::string(argv[1]) + "data/" + invalidBodyFilename);
  ::streamHelper invalidHeader(std::string(argv[1]) + "data/" + invalidHeaderFilename);
  ::streamHelper logo(std::string(argv[1]) + "data/" + logoFilename);
  ::streamHelper sphere1(std::string(argv[1]) + "data/" + sphere1Filename);
  ::streamHelper sphere2(std::string(argv[1]) + "data/" + sphere2Filename);
  ::streamHelper cube(std::string(argv[1]) + "data/" + cubeFilename);
  ::streamHelper world(std::string(argv[1]) + "data/" + worldFilename);

  // supports method
  test("not supported with empty buffer", !sce.supports(nullptr, 0));
  test("not supported with invalid header", !sce.supports(invalidHeader.Buffer.data(), invalidHeader.Size));
  test("supported with invalid body", sce.supports(invalidBody.Buffer.data(), invalidBody.Size));
  test("supported with default scene format", sce.supports(cube.Buffer.data(), cube.Size));
  test("supported with full scene format", sce.supports(logo.Buffer.data(), logo.Size));

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

  return test.result();
}
