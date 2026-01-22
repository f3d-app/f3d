#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <camera.h>
#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

int TestSDKDynamicUpDirection([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  f3d::camera& cam = win.getCamera();

  win.setSize(300, 300);

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  f3d::vector3_t initialUp = cam.getViewUp();

  test("initial up direction is +Y", TestSDKHelpers::CompareVec(initialUp, { 0, 1, 0 }));

  opt.scene.up_direction = { 0, 0, 1 };

  win.render();

  f3d::vector3_t newUp = cam.getViewUp();

  test("camera view up rotated to +Z", TestSDKHelpers::CompareVec(newUp, { 0, 0, 1 }));

  opt.scene.up_direction = { 1, 0, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated to +X", TestSDKHelpers::CompareVec(newUp, { 1, 0, 0 }));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated back to +Y", TestSDKHelpers::CompareVec(newUp, { 0, 1, 0 }));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();
  newUp = cam.getViewUp();
  test("setting same direction doesn't change anything", TestSDKHelpers::CompareVec(newUp, { 0, 1, 0 }));

  return test.result();
}
