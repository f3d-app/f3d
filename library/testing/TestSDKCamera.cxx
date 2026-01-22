#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <camera.h>
#include <engine.h>
#include <log.h>
#include <window.h>

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

int TestSDKCamera([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::camera& cam = win.getCamera();

  // check coordinates conversion
  f3d::point3_t point = { 0.1, 0.1, 0.1 };
  f3d::point3_t pointDC = win.getDisplayFromWorld(point);
  test("coordinates conversion", point, win.getWorldFromDisplay(pointDC),
    TestSDKHelpers::ComparePoint);

  // Test position
  f3d::point3_t testPos = { 0., 0., 10. };
  f3d::point3_t pos = cam.setPosition(testPos).getPosition();
  test("set/get position", pos, testPos, TestSDKHelpers::ComparePoint);

  // Test focal point
  f3d::point3_t testFoc = { 0., 0., -1. };
  f3d::point3_t foc = cam.setFocalPoint(testFoc).getFocalPoint();
  test("set/get focal point", foc, testFoc, TestSDKHelpers::ComparePoint);

  // Test view up
  f3d::vector3_t testUp = { 1., 0., 0. };
  f3d::vector3_t up = cam.setViewUp(testUp).getViewUp();
  test("set/get view up", up, testUp, TestSDKHelpers::CompareVec);

  // Test view angle
  f3d::angle_deg_t testAngle = 20;
  f3d::angle_deg_t angle = cam.setViewAngle(testAngle).getViewAngle();
  test("set/get view angle", angle, testAngle, TestSDKHelpers::CompareDouble);

  // Test azimuth
  cam.azimuth(90);
  f3d::point3_t expectedPos = { 0., -11., -1. };
  f3d::point3_t expectedFoc = { 0., 0., -1. };
  f3d::vector3_t expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("azimuth method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("azimuth method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("azimuth method up", up, expectedUp, TestSDKHelpers::CompareVec);

  // Test roll
  cam.roll(90);
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("roll method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("roll method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("roll method up", up, expectedUp, TestSDKHelpers::CompareVec);

  // Test yaw
  cam.yaw(90);
  expectedFoc = { 11., -11., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("yaw method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("yaw method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("yaw method up", up, expectedUp, TestSDKHelpers::CompareVec);

  // Test elevation
  cam.elevation(90);
  expectedPos = { 11., -11., -12. };
  expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("elevation method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("elevation method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("elevation method up", up, expectedUp, TestSDKHelpers::CompareVec);

  // Test pitch
  cam.pitch(90);
  expectedFoc = { 22., -11., -12. };
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("pitch method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("pitch method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("pitch method up", up, expectedUp, TestSDKHelpers::CompareVec);

  // Test dolly
  cam.dolly(10);
  expectedPos = { 20.9, -11., -12. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("dolly method position", pos, expectedPos, TestSDKHelpers::ComparePoint);
  test("dolly method focal point", foc, expectedFoc, TestSDKHelpers::ComparePoint);
  test("dolly method up", up, expectedUp, TestSDKHelpers::CompareVec);

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.pan(1, 2);
  test("pos after pan", cam.getPosition(), f3d::point3_t{ 0, 4, 3 }, TestSDKHelpers::ComparePoint);
  test(
    "foc after pan", cam.getFocalPoint(), f3d::point3_t{ 0, 4, 13 }, TestSDKHelpers::ComparePoint);
  test("up after pan", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, TestSDKHelpers::CompareVec);

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, -2, 3 });
  cam.setViewUp({ 0, 0, 1 });
  cam.pan(3, 4, 5);
  test(
    "pos after pan", cam.getPosition(), f3d::point3_t{ -2, -3, 7 }, TestSDKHelpers::ComparePoint);
  test(
    "foc after pan", cam.getFocalPoint(), f3d::point3_t{ -2, -7, 7 }, TestSDKHelpers::ComparePoint);
  test("up after pan", cam.getViewUp(), f3d::vector3_t{ 0, 0, 1 });

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.setViewAngle(25);
  cam.zoom(1.5);
  test("pos after zoom", cam.getPosition(), f3d::point3_t{ 1, 2, 3 }, TestSDKHelpers::ComparePoint);
  test(
    "foc after zoom", cam.getFocalPoint(), f3d::point3_t{ 1, 2, 13 }, TestSDKHelpers::ComparePoint);
  test("up after zoom", cam.getViewUp(), f3d::vector3_t{ 0, 1, 0 }, TestSDKHelpers::CompareVec);
  test("angle after zoom", cam.getViewAngle(), 25 / 1.5, TestSDKHelpers::CompareDouble);

  cam.setPosition({ 1, 0, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 1", cam.getPosition(),
    f3d::point3_t{ 1, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 1", cam.getFocalPoint(),
    f3d::point3_t{ 0, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("up when cross product of pos->foc and up is 0 - test 1", cam.getViewUp(),
    f3d::vector3_t{ 0, 1, 0 }, TestSDKHelpers::CompareVec);

  cam.setPosition({ 0, 1, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 1, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 2", cam.getPosition(),
    f3d::point3_t{ 0, 1, 0 }, TestSDKHelpers::ComparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 2", cam.getFocalPoint(),
    f3d::point3_t{ 0, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("up when cross product of pos->foc and up is 0 - test 2", cam.getViewUp(),
    f3d::vector3_t{ 1, 0, 0 }, TestSDKHelpers::CompareVec);

  cam.setPosition({ 0, 0, 1 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 0, 1 });
  test("pos when cross product of pos->foc and up is 0 - test 3", cam.getPosition(),
    f3d::point3_t{ 0, 0, 1 }, TestSDKHelpers::ComparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 3", cam.getFocalPoint(),
    f3d::point3_t{ 0, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("up when cross product of pos->foc and up is 0 - test 3", cam.getViewUp(),
    f3d::vector3_t{ 1, 0, 0 }, TestSDKHelpers::CompareVec);

  cam.setPosition({ 5, 0, 0 });
  cam.setFocalPoint({ 1, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 4", cam.getPosition(),
    f3d::point3_t{ 5, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("foc when cross product of pos->foc and up is 0 - test 4", cam.getFocalPoint(),
    f3d::point3_t{ 1, 0, 0 }, TestSDKHelpers::ComparePoint);
  test("up when cross product of pos->foc and up is 0 - test 4", cam.getViewUp(),
    f3d::vector3_t{ 0, 1, 0 }, TestSDKHelpers::CompareVec);

  return test.result();
}
