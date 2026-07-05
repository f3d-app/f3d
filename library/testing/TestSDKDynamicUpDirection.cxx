#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <camera.h>
#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

int TestSDKDynamicUpDirection([[maybe_unused]] int argc, char* argv[])
{
  PseudoUnitTest test;

  std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  f3d::camera& cam = win.getCamera();

  win.setSize(300, 300);

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  f3d::vector3_t initialUp = cam.getViewUp();

  test("initial up direction is +Y", initialUp, approx(f3d::vector3_t({ 0, 1, 0 })));

  opt.scene.up_direction = { 0, 0, 1 };

  win.render();

  f3d::vector3_t newUp = cam.getViewUp();

  test("camera view up rotated to +Z", newUp, approx(f3d::vector3_t({ 0, 0, 1 })));

  opt.scene.up_direction = { 1, 0, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated to +X", newUp, approx(f3d::vector3_t({ 1, 0, 0 })));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated back to +Y", newUp, approx(f3d::vector3_t({ 0, 1, 0 })));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();
  newUp = cam.getViewUp();
  test(
    "setting same direction doesn't change anything", newUp, approx(f3d::vector3_t({ 0, 1, 0 })));

  return test.result();
}
