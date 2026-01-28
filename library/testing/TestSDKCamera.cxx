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
  test("coordinates conversion", point, approx(win.getWorldFromDisplay(pointDC)));

  // Test position
  f3d::point3_t testPos = { 0., 0., 10. };
  f3d::point3_t pos = cam.setPosition(testPos).getPosition();
  test("set/get position", pos, testPos);

  // Test focal point
  f3d::point3_t testFoc = { 0., 0., -1. };
  f3d::point3_t foc = cam.setFocalPoint(testFoc).getFocalPoint();
  test("set/get focal point", foc, testFoc);

  // Test view up
  f3d::vector3_t testUp = { 1., 0., 0. };
  f3d::vector3_t up = cam.setViewUp(testUp).getViewUp();
  test("set/get view up", up, testUp);

  // Test view angle
  f3d::angle_deg_t testAngle = 20;
  f3d::angle_deg_t angle = cam.setViewAngle(testAngle).getViewAngle();
  test("set/get view angle", angle, testAngle);

  // Test azimuth
  cam.azimuth(90);
  f3d::point3_t expectedPos = { 0., -11., -1. };
  f3d::point3_t expectedFoc = { 0., 0., -1. };
  f3d::vector3_t expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("azimuth method position", pos, approx(expectedPos));
  test("azimuth method focal point", foc, approx(expectedFoc));
  test("azimuth method up", up, approx(expectedUp));

  // Test roll
  cam.roll(90);
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("roll method position", pos, approx(expectedPos));
  test("roll method focal point", foc, approx(expectedFoc));
  test("roll method up", up, approx(expectedUp));

  // Test yaw
  cam.yaw(90);
  expectedFoc = { 11., -11., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("yaw method position", pos, approx(expectedPos));
  test("yaw method focal point", foc, approx(expectedFoc));
  test("yaw method up", up, approx(expectedUp));

  // Test elevation
  cam.elevation(90);
  expectedPos = { 11., -11., -12. };
  expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("elevation method position", pos, approx(expectedPos));
  test("elevation method focal point", foc, approx(expectedFoc));
  test("elevation method up", up, approx(expectedUp));

  // Test pitch
  cam.pitch(90);
  expectedFoc = { 22., -11., -12. };
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("pitch method position", pos, approx(expectedPos));
  test("pitch method focal point", foc, approx(expectedFoc));
  test("pitch method up", up, approx(expectedUp));

  // Test dolly
  cam.dolly(10);
  expectedPos = { 20.9, -11., -12. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  test("dolly method position", pos, approx(expectedPos));
  test("dolly method focal point", foc, approx(expectedFoc));
  test("dolly method up", up, approx(expectedUp));

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.pan(1, 2);
  test("pos after pan", cam.getPosition(), { 0, 4, 3 });
  test("foc after pan", cam.getFocalPoint(), { 0, 4, 13 });
  test("up after pan", cam.getViewUp(), { 0, 1, 0 });

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, -2, 3 });
  cam.setViewUp({ 0, 0, 1 });
  cam.pan(3, 4, 5);
  test("pos after pan", cam.getPosition(), { -2, -3, 7 });
  test("foc after pan", cam.getFocalPoint(), { -2, -7, 7 });
  test("up after pan", cam.getViewUp(), { 0, 0, 1 });

  cam.setPosition({ 1, 2, 3 });
  cam.setFocalPoint({ 1, 2, 13 });
  cam.setViewUp({ 0, 1, 0 });
  cam.setViewAngle(25);
  cam.zoom(1.5);
  test("pos after zoom", cam.getPosition(), { 1, 2, 3 });
  test("foc after zoom", cam.getFocalPoint(), { 1, 2, 13 });
  test("up after zoom", cam.getViewUp(), { 0, 1, 0 });
  test("angle after zoom", cam.getViewAngle(), 25 / 1.5);

  cam.setPosition({ 1, 0, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 1", cam.getPosition(), { 1, 0, 0 });
  test("foc when cross product of pos->foc and up is 0 - test 1", cam.getFocalPoint(), { 0, 0, 0 });
  test("up when cross product of pos->foc and up is 0 - test 1", cam.getViewUp(), { 0, 1, 0 });

  cam.setPosition({ 0, 1, 0 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 1, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 2", cam.getPosition(), { 0, 1, 0 });
  test("foc when cross product of pos->foc and up is 0 - test 2", cam.getFocalPoint(), { 0, 0, 0 });
  test("up when cross product of pos->foc and up is 0 - test 2", cam.getViewUp(), { 1, 0, 0 });

  cam.setPosition({ 0, 0, 1 });
  cam.setFocalPoint({ 0, 0, 0 });
  cam.setViewUp({ 0, 0, 1 });
  test("pos when cross product of pos->foc and up is 0 - test 3", cam.getPosition(), { 0, 0, 1 });
  test("foc when cross product of pos->foc and up is 0 - test 3", cam.getFocalPoint(), { 0, 0, 0 });
  test("up when cross product of pos->foc and up is 0 - test 3", cam.getViewUp(), { 1, 0, 0 });

  cam.setPosition({ 5, 0, 0 });
  cam.setFocalPoint({ 1, 0, 0 });
  cam.setViewUp({ 1, 0, 0 });
  test("pos when cross product of pos->foc and up is 0 - test 4", cam.getPosition(), { 5, 0, 0 });
  test("foc when cross product of pos->foc and up is 0 - test 4", cam.getFocalPoint(), { 1, 0, 0 });
  test("up when cross product of pos->foc and up is 0 - test 4", cam.getViewUp(), { 0, 1, 0 });

  // Test world azimuth, elevation and distance
  f3d::options opt = eng.getOptions();
  opt.scene.up_direction = { 0, 0, 1 };
  eng.setOptions(opt);
  win.render();

  // Case 1: Horizontal view, +Y forward
  cam.setPosition({ 0., -10., 0. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  test("getDistance (horizontal)", cam.getDistance(), approx(10.0));
  test("getWorldElevation (horizontal)", cam.getWorldElevation(), approx(0.0));
  test("getWorldAzimuth (horizontal)", cam.getWorldAzimuth(), approx(-90.0));

  // Case 2: Positive elevation (+Z)
  cam.resetToDefault();
  cam.setPosition({ 0., -10., -10. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  test("getDistance (positive elevation)", cam.getDistance(), approx(std::sqrt(200.0)));
  test("getWorldElevation (positive elevation)", cam.getWorldElevation(), approx(-45.0));

  // Case 3: Negative elevation (-Z)
  cam.resetToDefault();
  cam.setPosition({ 0., -10., 10. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  test("getWorldElevation (negative elevation)", cam.getWorldElevation(), approx(45.0));

  // Case 4: Zero-length direction vector
  cam.resetToDefault();
  cam.setPosition({ 0., 0., 0. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  test("getDistance (zero vector)", cam.getDistance(), approx(0.0));
  test("getWorldAzimuth (zero vector)", cam.getWorldAzimuth(), approx(0.0));
  test("getWorldElevation (zero vector)", cam.getWorldElevation(), approx(0.0));

  // Case 5: Custom environment up (+X)
  opt.scene.up_direction = { 1, 0, 0 };
  eng.setOptions(opt);
  win.render();

  cam.resetToDefault();
  cam.setPosition({ -10., 0., 0. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 1., 0., 0. });

  test("getWorldElevation (custom up)", cam.getWorldElevation(), approx(-90.0));

  // Case 6: Camera operations before calculation (typical usage)
  cam.resetToDefault();
  cam.setPosition({ 5., -7., 3. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  const double initialDistance = cam.getDistance();

  cam.azimuth(37.0);
  cam.elevation(-22.0);

  test("getDistance invariant after azimuth/elevation",
       cam.getDistance(), approx(initialDistance));

  test("getWorldAzimuth finite after camera operations",
       std::isfinite(cam.getWorldAzimuth()));

  test("getWorldElevation finite after camera operations",
       std::isfinite(cam.getWorldElevation()));
  return test.result();
}
