#include "PseudoUnitTest.h"

#include <camera.h>
#include <engine.h>
#include <log.h>
#include <window.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>


int TestSDKCamera([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  const auto compareDouble = [](double a, double b) { return std::fabs(a - b) < 128 * std::numeric_limits<double>::epsilon(); };

  const auto compareVec = [compareDouble](const f3d::vector3_t& vec1, const f3d::vector3_t& vec2)
  {
    return compareDouble(vec1[0], vec2[0]) && compareDouble(vec1[1], vec2[1]) &&
      compareDouble(vec1[2], vec2[2]);
  };

  const auto comparePoint = [compareDouble](const f3d::point3_t& vec1, const f3d::point3_t& vec2)
  {
    return compareDouble(vec1[0], vec2[0]) && compareDouble(vec1[1], vec2[1]) &&
      compareDouble(vec1[2], vec2[2]);
  };

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::camera& cam = win.getCamera();

  // check coordinates conversion
  f3d::point3_t point = { 0.1, 0.1, 0.1 };
  f3d::point3_t pointDC = win.getDisplayFromWorld(point);
  test("coordinates conversion", point, win.getWorldFromDisplay(pointDC), comparePoint);

  // Test position
  f3d::point3_t testPos = { 0., 0., 10. };
  f3d::point3_t pos = cam.setPosition(testPos).getPosition();
  test("set/get position", pos, testPos, comparePoint);

  // Test focal point
  f3d::point3_t testFoc = { 0., 0., -1. };
  f3d::point3_t foc = cam.setFocalPoint(testFoc).getFocalPoint();
  test("set/get focal point", foc, testFoc, comparePoint);

  // Test view up
  f3d::vector3_t testUp = { 1., 0., 0. };
  f3d::vector3_t up = cam.setViewUp(testUp).getViewUp();
  test("set/get view up", up, testUp, compareVec);

  // Test view angle
  f3d::angle_deg_t testAngle = 20;
  f3d::angle_deg_t angle = cam.setViewAngle(testAngle).getViewAngle();
  test("set/get view angle", angle, testAngle, compareDouble);

  // Test azimuth
  cam.azimuth(90);
  f3d::point3_t expectedPos = { 0., -11., -1. };
  f3d::point3_t expectedFoc = { 0., 0., -1. };
  f3d::vector3_t expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("azimuth method position", pos, expectedPos, comparePoint);
  test("azimuth method focal point", foc, expectedFoc, comparePoint);
  test("azimuth method up", up, expectedUp, compareVec);

  // Test roll
  cam.roll(90);
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("roll method position", pos, expectedPos, comparePoint);
  test("roll method focal point", foc, expectedFoc, comparePoint);
  test("roll method up", up, expectedUp, compareVec);

  // Test yaw
  cam.yaw(90);
  expectedFoc = { 11., -11., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("yaw method position", pos, expectedPos, comparePoint);
  test("yaw method focal point", foc, expectedFoc, comparePoint);
  test("yaw method up", up, expectedUp, compareVec);

  // Test elevation
  cam.elevation(90);
  expectedPos = { 11., -11., -12. };
  expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("elevation method position", pos, expectedPos, comparePoint);
  test("elevation method focal point", foc, expectedFoc, comparePoint);
  test("elevation method up", up, expectedUp, compareVec);

  // Test pitch
  cam.pitch(90);
  expectedFoc = { 22., -11., -12. };
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("pitch method position", pos, expectedPos, comparePoint);
  test("pitch method focal point", foc, expectedFoc, comparePoint);
  test("pitch method up", up, expectedUp, compareVec);

  // Test dolly
  cam.dolly(10);
  expectedPos = { 20.9, -11., -12. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("dolly method position", pos, expectedPos, comparePoint);
  test("dolly method focal point", foc, expectedFoc, comparePoint);
  test("dolly method up", up, expectedUp, compareVec);

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.pan(1, 2);
  test("pos after pan", cam.getPosition(), f3d::point3_t{ 0, 4, 3 }, comparePoint);
  test("foc after pan", cam.getFocalPoint(), f3d::point3_t{ 0, 4, 13 }, comparePoint);
  test("up after pan", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, compareVec);

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, -2, 3 });
  cam.setViewUp({ 0, 0, 1 });
  cam.pan(3, 4, 5);
  test("pos after pan", cam.getPosition(), f3d::point3_t{ -2, -3, 7 }, comparePoint);
  test("foc after pan", cam.getFocalPoint(), f3d::point3_t{ -2, -7, 7 }, comparePoint);
  test("up after pan", cam.getViewUp(), f3d::vector3_t{ 0, 0, 1 });

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.setViewAngle(25);
  cam.zoom(1.5);
  test("pos after zoom", cam.getPosition(), f3d::point3_t{ 1, 2, 3 }, comparePoint);
  test("foc after zoom", cam.getFocalPoint(), f3d::point3_t{ 1, 2, 13 }, comparePoint);
  test("up after zoom", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, compareVec);
  test("angle after zoom", cam.getViewAngle(), 25 / 1.5, compareDouble);

  cam.setPosition({ 1, 0, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 1", cam.getPosition(), f3d::point3_t{ 1, 0, 0 }, comparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 1", cam.getFocalPoint(), f3d::point3_t{ 0, 0, 0 }, comparePoint);
  test("up when cross product of pos->foc and up is 0 - test 1", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, compareVec);

  cam.setPosition({ 0, 1, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 1, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 2", cam.getPosition(), f3d::point3_t{ 0, 1, 0 }, comparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 2", cam.getFocalPoint(), f3d::point3_t{ 0, 0, 0 }, comparePoint);
  test("up when cross product of pos->foc and up is 0 - test 2", cam.getViewUp(), f3d::vector3_t{ 1, 0, 0 }, compareVec);

  cam.setPosition({ 0, 0, 1 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 0, 1 });
  test("pos when cross product of pos->foc and up is 0 - test 3", cam.getPosition(), f3d::point3_t{ 0, 0, 1 }, comparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 3", cam.getFocalPoint(), f3d::point3_t{ 0, 0, 0 }, comparePoint);
  test("up when cross product of pos->foc and up is 0 - test 3", cam.getViewUp(), f3d::vector3_t{ 1, 0, 0 }, compareVec);

  cam.setPosition({ 5, 0, 0 });
  cam.setFocalPoint({ 1, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 4", cam.getPosition(), f3d::point3_t{ 5, 0, 0 }, comparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 4", cam.getFocalPoint(), f3d::point3_t{ 1, 0, 0 }, comparePoint);
  test("up when cross product of pos->foc and up is 0 - test 4", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, compareVec);

  return test.result();
}
