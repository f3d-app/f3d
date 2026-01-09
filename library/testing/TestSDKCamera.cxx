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

  // Test getters: world azimuth / elevation / distance
  // Case 1: Horizontal view
  cam.setPosition({ 0., -11., -1. });
  cam.setFocalPoint({ 0., 0., -1. });
  cam.setViewUp({ 1., 0., 0. });

  double azimuth = cam.getWorldAzimuth();
  double elevation = cam.getWorldElevation();
  double distance = cam.getDistance();

  if (!compareDouble(distance, 11.0))
  {
    std::cerr << "getDistance (horizontal) is not behaving as expected: " << distance << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth (horizontal) is not behaving as expected: " << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, 0.0))
  {
    std::cerr << "getWorldElevation (horizontal) is not behaving as expected: "
              << elevation << "\n";
    return EXIT_FAILURE;
  }

  // Case 2: Positive elevation (+45 deg)
  cam.setPosition({ 0., -11., -1. });
  cam.setFocalPoint({ 0., 0., 10. });
  cam.setViewUp({ 1., 0., 0. });

  azimuth = cam.getWorldAzimuth();
  elevation = cam.getWorldElevation();
  distance = cam.getDistance();

  if (!compareDouble(distance, std::sqrt(11.0 * 11.0 + 11.0 * 11.0)))
  {
    std::cerr << "getDistance (positive elevation) is not behaving as expected: "
              << distance << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth (positive elevation) is not behaving as expected: "
              << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, 45.0))
  {
    std::cerr << "getWorldElevation (positive elevation) is not behaving as expected: "
              << elevation << "\n";
    return EXIT_FAILURE;
  }

  // Case 3: Negative elevation (-45 deg)
  cam.setPosition({ 0., -11., 10. });
  cam.setFocalPoint({ 0., 0., -1. });
  cam.setViewUp({ 1., 0., 0. });

  azimuth = cam.getWorldAzimuth();
  elevation = cam.getWorldElevation();
  distance = cam.getDistance();

  if (!compareDouble(distance, std::sqrt(11.0 * 11.0 + 11.0 * 11.0)))
  {
    std::cerr << "getDistance (negative elevation) is not behaving as expected: "
              << distance << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth (negative elevation) is not behaving as expected: "
              << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, -45.0))
  {
    std::cerr << "getWorldElevation (negative elevation) is not behaving as expected: " << elevation
              << "\n";
    return EXIT_FAILURE;
  }

  // Case 4: Custom up direction (non-Z up)
  cam.setPosition({ -11., 0., 0. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 1., 0. });

  azimuth = cam.getWorldAzimuth();
  elevation = cam.getWorldElevation();
  distance = cam.getDistance();

  if (!compareDouble(distance, 11.0))
  {
    std::cerr << "getDistance (custom up) is not behaving as expected: " << distance << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth (custom up) is not behaving as expected: " << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, 0.0))
  {
    std::cerr << "getWorldElevation (custom up) is not behaving as expected: " << elevation << "\n";
    return EXIT_FAILURE;
  }

  // Case 5: position equals focal point (zero direction vector)
  cam.setPosition({ 0., 0., 0. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 1., 0. });

  azimuth = cam.getWorldAzimuth();
  elevation = cam.getWorldElevation();

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth with zero direction vector should return 0: " << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, 0.0))
  {
    std::cerr << "getWorldElevation with zero direction vector should return 0: " << elevation
              << "\n";
    return EXIT_FAILURE;
  }

  // Case 6: view direction parallel to environment up
  cam.setPosition({ 0., 0., -1. });
  cam.setFocalPoint({ 0., 0., 0. });
  cam.setViewUp({ 0., 0., 1. });

  azimuth = cam.getWorldAzimuth();
  elevation = cam.getWorldElevation();

  if (!compareDouble(azimuth, 0.0))
  {
    std::cerr << "getWorldAzimuth with forward parallel to up should return 0: " << azimuth << "\n";
    return EXIT_FAILURE;
  }

  if (!compareDouble(elevation, 90.0))
  {
    std::cerr << "getWorldElevation with forward parallel to up should be 90: " << elevation
              << "\n";
    return EXIT_FAILURE;
  }

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

  return test.result();
}
