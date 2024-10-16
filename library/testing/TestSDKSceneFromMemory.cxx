#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

int TestSDKSceneFromMemory(int argc, char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow().setSize(300, 300);

  std::string texturePath = std::string(argv[1]) + "data/world.png";
  eng.getOptions().model.color.texture = texturePath;

  // Add mesh with invalid number of points
  test.expect<f3d::scene::load_failure_exception>("add mesh with invalid number of points", [&]() {
    sce.add(f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2 } });
  });

  // Add mesh with empty points
  test.expect<f3d::scene::load_failure_exception>(
    "add mesh with empty points", [&]() { sce.add(f3d::mesh_t{}); });

  // Add mesh with invalid number of cell indices
  test.expect<f3d::scene::load_failure_exception>(
    "add mesh with invalid number of cell indices", [&]() {
      sce.add(f3d::mesh_t{
        { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2, 3 } });
    });

  // Add mesh with invalid vertex index
  test.expect<f3d::scene::load_failure_exception>("add mesh with invalid vertex index", [&]() {
    sce.add(
      f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 4 } });
  });

  // Add mesh with invalid normals
  test.expect<f3d::scene::load_failure_exception>("add mesh with invalid normals", [&]() {
    sce.add(f3d::mesh_t{
      { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, { 1.f }, {}, { 3 }, { 0, 1, 2, 4 } });
  });

  // Add mesh with invalid texture coordinates
  test.expect<f3d::scene::load_failure_exception>(
    "add mesh with invalid texture coordinates", [&]() {
      sce.add(f3d::mesh_t{
        { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, { 1.f }, { 3 }, { 0, 1, 2, 4 } });
    });

  // Add mesh from memory and render it
  test("add mesh from memory", [&]() {
    sce.add(f3d::mesh_t{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f },
      { 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f }, { 3, 3 }, { 0, 1, 2, 1, 3, 2 } });
  });

  // Render test
  test("render mesh from memory", [&]() {
    if (!TestSDKHelpers::RenderTest(
          win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemory"))
    {
      throw "rendering test failed";
    }
  });

  return test.result();
}
