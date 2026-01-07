#include "PseudoUnitTest.h"

#include <camera.h>
#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <cmath>

namespace
{
bool compareDouble(double a, double b, double epsilon = 1e-6)
{
  return std::fabs(a - b) < epsilon;
}

bool compareVec(const f3d::vector3_t& vec1, const f3d::vector3_t& vec2, double epsilon = 1e-6)
{
  return compareDouble(vec1[0], vec2[0], epsilon) && compareDouble(vec1[1], vec2[1], epsilon) &&
    compareDouble(vec1[2], vec2[2], epsilon);
}
}

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

  test("initial up direction is +Y", compareVec(initialUp, { 0, 1, 0 }));

  opt.scene.up_direction = { 0, 0, 1 };

  win.render();

  f3d::vector3_t newUp = cam.getViewUp();

  test("camera view up rotated to +Z", compareVec(newUp, { 0, 0, 1 }));

  opt.scene.up_direction = { 1, 0, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated to +X", compareVec(newUp, { 1, 0, 0 }));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();

  newUp = cam.getViewUp();
  test("camera view up rotated back to +Y", compareVec(newUp, { 0, 1, 0 }));

  opt.scene.up_direction = { 0, 1, 0 };
  win.render();
  newUp = cam.getViewUp();
  test("setting same direction doesn't change anything", compareVec(newUp, { 0, 1, 0 }));

  return test.result();
}
