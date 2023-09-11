#include <engine.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <iostream>

int TestSDKLoadFromMemory(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::window& win = eng.getWindow().setSize(300, 300);

  // Load invalid number of points
  try
  {
    eng.getLoader().loadGeometry({ 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f }, { 3 }, { 0, 1, 2 });
    std::cerr << "Should throw: invalid number of points" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Load invalid number of points
  try
  {
    eng.getLoader().loadGeometry(
      { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, { 3 }, { 0, 1, 2, 3 });
    std::cerr << "Should throw: invalid number of cell indices" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Load invalid with invalid index
  try
  {
    eng.getLoader().loadGeometry(
      { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, { 3 }, { 0, 1, 2, 4 });
    std::cerr << "Should throw: invalid vertex index" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
  }

  // Load from memory (valid)
  try
  {
    eng.getLoader().loadGeometry({ 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 3, 3 }, { 0, 1, 2, 1, 3, 2 });

    win.renderToImage().save("/tmp/out.png");
  }
  catch (const f3d::loader::load_failure_exception& ex)
  {
    std::cerr << "Unexpected loadMemory failure" << std::endl;
    return EXIT_FAILURE;
  }

  if (!TestSDKHelpers::RenderTest(
        win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKLoadMemory"))
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
