#include <camera.h>
#include <engine.h>
#include <loader.h>
#include <window.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

// TODO these methods should be put in types.h at some point.
// https://github.com/f3d-app/f3d/issues/361
bool compareDouble(double a, double b)
{
  return std::fabs(a - b) < 128 * std::numeric_limits<double>::epsilon();
}

bool compareVec(const f3d::vector3_t& vec1, const f3d::vector3_t& vec2)
{
  return compareDouble(vec1[0], vec2[0]) && compareDouble(vec1[1], vec2[1]) &&
    compareDouble(vec1[2], vec2[2]);
}

bool comparePoint(const f3d::point3_t& vec1, const f3d::point3_t& vec2)
{
  return compareDouble(vec1[0], vec2[0]) && compareDouble(vec1[1], vec2[1]) &&
    compareDouble(vec1[2], vec2[2]);
}

class testFailure : public std::runtime_error
{
public:
  explicit testFailure(const std::string& what = "")
    : std::runtime_error(what)
  {
  }
};

void checkVec3(const std::array<double, 3>& actual, const std::array<double, 3>& expected,
  const std::string& label)
{
  if (!compareDouble(actual[0], expected[0]) || !compareDouble(actual[1], expected[1]) ||
    !compareDouble(actual[2], expected[2]))
  {
    std::stringstream ss;
    ss << label << ": ";
    ss << std::setprecision(12);
    ss << "(" << actual[0] << "," << actual[1] << "," << actual[2] << ")";
    ss << " != ";
    ss << "(" << expected[0] << "," << expected[1] << "," << expected[2] << ")";
    throw testFailure(ss.str());
  }
}

void checkDouble(const double actual, const double expected, const std::string& label)
{
  if (!compareDouble(actual, expected))
  {
    std::stringstream ss;
    ss << label << ": ";
    ss << std::setprecision(12);
    ss << actual << " != " << expected;
    throw testFailure(ss.str());
  }
}

int TestSDKCamera(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::window& win = eng.getWindow();
  f3d::camera& cam = win.getCamera();

  // check coordinates conversion
  f3d::point3_t point = { 0.1, 0.1, 0.1 };
  f3d::point3_t pointDC = win.getDisplayFromWorld(point);

  if (!comparePoint(point, win.getWorldFromDisplay(pointDC)))
  {
    std::cerr << "coordinates conversion is not behaving as expected" << std::endl;
    return EXIT_FAILURE;
  }

  // Test position
  f3d::point3_t testPos = { 0., 0., 10. };
  f3d::point3_t pos = cam.setPosition(testPos).getPosition();
  if (pos != testPos)
  {
    std::cerr << "set/get position is not behaving as expected: " << pos[0] << "," << pos[1] << ","
              << pos[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test focal point
  f3d::point3_t testFoc = { 0., 0., -1. };
  f3d::point3_t foc = cam.setFocalPoint(testFoc).getFocalPoint();
  if (foc != testFoc)
  {
    std::cerr << "set/get focal point is not behaving as expected: " << foc[0] << "," << foc[1]
              << "," << foc[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test view up
  f3d::vector3_t testUp = { 1., 0., 0. };
  f3d::vector3_t up = cam.setViewUp(testUp).getViewUp();
  if (up != testUp)
  {
    std::cerr << "set/get view up is not behaving as expected: " << up[0] << "," << up[1] << ","
              << up[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test view angle
  f3d::angle_deg_t testAngle = 20;
  f3d::angle_deg_t angle = cam.setViewAngle(testAngle).getViewAngle();
  if (angle != testAngle)
  {
    std::cerr << "set/get view angle is not behaving as expected: " << angle << std::endl;
    return EXIT_FAILURE;
  }

  // Test azimuth
  cam.azimuth(90);
  f3d::point3_t expectedPos = { 0., -11., -1. };
  f3d::point3_t expectedFoc = { 0., 0., -1. };
  f3d::vector3_t expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Azimuth is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test roll
  cam.roll(90);
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Roll is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test yaw
  cam.yaw(90);
  expectedFoc = { 11., -11., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Yaw is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test getAzimuth
  double azimuth = cam.getAzimuth();
  checkDouble(azimuth, 0.0, "getAzimuth");


  // Test getYaw (same value as azimuth here)
  double yaw = cam.getYaw();
  checkDouble(yaw, 90.0, "getYaw");

  // Test getElevation
  double elevation = cam.getElevation();
  checkDouble(elevation, 0.0, "getElevation");

  // Test elevation
  cam.elevation(90);
  expectedPos = { 11., -11., -12. };
  expectedUp = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Elevation is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test pitch
  cam.pitch(90);
  expectedFoc = { 22., -11., -12. };
  expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Pitch is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test dolly
  cam.dolly(10);
  expectedPos = { 20.9, -11., -12. };
  pos = cam.getPosition();
  foc = cam.getFocalPoint();
  up = cam.getViewUp();
  if (!comparePoint(pos, expectedPos) || !comparePoint(foc, expectedFoc) ||
    !compareVec(up, expectedUp))
  {
    std::cerr << "Dolly is not behaving as expected: " << std::endl;
    std::cerr << std::setprecision(12) << "position: " << pos[0] << "," << pos[1] << "," << pos[2]
              << std::endl;
    std::cerr << std::setprecision(12) << "focal point: " << foc[0] << "," << foc[1] << ","
              << foc[2] << std::endl;
    std::cerr << std::setprecision(12) << "view up: " << up[0] << "," << up[1] << "," << up[2]
              << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    cam.setPosition({ 1, 2, 3 });
    cam.setFocalPoint({ 1, 2, 13 });
    cam.setViewUp({ 0, 1, 0 });
    cam.pan(1, 2);
    checkVec3(cam.getPosition(), { 0, 4, 3 }, "pos after pan");
    checkVec3(cam.getFocalPoint(), { 0, 4, 13 }, "foc after pan");
    checkVec3(cam.getViewUp(), { 0, 1, 0 }, "up after pan");

    cam.setPosition({ 1, 2, 3 });
    cam.setFocalPoint({ 1, -2, 3 });
    cam.setViewUp({ 0, 0, 1 });
    cam.pan(3, 4, 5);
    checkVec3(cam.getPosition(), { -2, -3, 7 }, "pos after pan");
    checkVec3(cam.getFocalPoint(), { -2, -7, 7 }, "foc after pan");
    checkVec3(cam.getViewUp(), { 0, 0, 1 }, "up after pan");

    cam.setPosition({ 1, 2, 3 });
    cam.setFocalPoint({ 1, 2, 13 });
    cam.setViewUp({ 0, 1, 0 });
    cam.setViewAngle(25);
    cam.zoom(1.5);
    checkVec3(cam.getPosition(), { 1, 2, 3 }, "pos after zoom");
    checkVec3(cam.getFocalPoint(), { 1, 2, 13 }, "foc after zoom");
    checkVec3(cam.getViewUp(), { 0, 1, 0 }, "up after zoom");
    checkDouble(cam.getViewAngle(), 25 / 1.5, "angle after zoom");
  }
  catch (testFailure& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
