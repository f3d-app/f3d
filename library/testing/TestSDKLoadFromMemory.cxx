#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <loader.h>
#include <log.h>
#include <window.h>

#include <iostream>

int TestSDKLoadFromMemory(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::loader& load = eng.getLoader();
  f3d::window& win = eng.getWindow().setSize(300, 300);

  std::string texturePath = std::string(argv[1]) + "data/world.png";
  eng.getOptions().model.color.texture = texturePath;

  // Add mesh with invalid number of points
  test.expect<f3d::loader::load_failure_exception>("add mesh with invalid number of points", [&]() {
    load.add(f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2 } });
  });

  // Add mesh with empty points
  test.expect<f3d::loader::load_failure_exception>(
    "add mesh with empty points", [&]() { load.add(f3d::mesh_t{}); });

  // Add mesh with invalid number of cell indices
  test.expect<f3d::loader::load_failure_exception>(
    "add mesh with invalid number of cell indices", [&]() {
      load.add(f3d::mesh_t{
        { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2, 3 } });
    });

  // Add mesh with invalid vertex index
  test.expect<f3d::loader::load_failure_exception>("add mesh with invalid vertex index", [&]() {
    load.add(
      f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 4 } });
  });

  // Add mesh with invalid normals
  test.expect<f3d::loader::load_failure_exception>("add mesh with invalid normals", [&]() {
    load.add(f3d::mesh_t{
      { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, { 1.f }, {}, { 3 }, { 0, 1, 2, 4 } });
  });

  // Add mesh with invalid texture coordinates
  test.expect<f3d::loader::load_failure_exception>(
    "add mesh with invalid texture coordinates", [&]() {
      load.add(f3d::mesh_t{
        { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, { 1.f }, { 3 }, { 0, 1, 2, 4 } });
    });

  // Add mesh from memory and render it
  test("add mesh from memory", [&]() {
    load.add(f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f },
      { 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f }, { 3, 3 }, { 0, 1, 2, 1, 3, 2 } });
  });

  // Render test
  test("render mesh from memory", [&]() {
    if (!TestSDKHelpers::RenderTest(
          win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKLoadMemory"))
    {
      throw "rendering test failed";
    }
  });

  return test.result();
}
