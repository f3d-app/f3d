#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <options.h>
#include <scene.h>

int TestSDKSceneFromBuffer([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();

  // Add empty buffer
  test("Add empty buffer", [&]() { sce.add(nullptr, 0); });

  char c = '\0';

  // Add buffer without setting reader
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer without setting reader", [&]() { sce.add(&c, 1); });

  // Add buffer with invalid reader
  opt.scene.force_reader = "INVALID";
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer with invalid reader", [&]() { sce.add(&c, 1); });

  // Add buffer with reader that doesn't support streams
  opt.scene.force_reader = "Nrrd";
  test.expect<f3d::scene::load_failure_exception>(
    "add buffer with reader that doesn't support streams", [&]() { sce.add(&c, 1); });

  return test.result();
}
