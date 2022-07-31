#include <camera.h>
#include <engine.h>
#include <loader.h>
#include <window.h>

#include <cmath>
#include <iomanip>
#include <iostream>

bool compareDouble(double a, double b)
{
  return std::fabs(a - b) < 128 * std::numeric_limits<double>::epsilon();
}

bool compareVec(f3d::camera::vector3_t vec1, f3d::camera::vector3_t vec2)
{
  return compareDouble(vec1[0], vec2[0]) && compareDouble(vec1[1], vec2[1]) &&
    compareDouble(vec1[2], vec2[2]);
}

bool compareMat(f3d::camera::matrix4_t mat1, f3d::camera::matrix4_t mat2)
{
  bool ret = true;
  for (size_t i = 0; i < mat1.size(); i++)
  {
    ret &= compareDouble(mat1[i], mat2[i]);
  }
  return ret;
}

int TestSDKCamera(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::window& win = eng.getWindow();
  f3d::camera& cam = win.getCamera();

  // Test position
  f3d::camera::vector3_t testPos = { 0., 0., 10. };
  f3d::camera::vector3_t pos = cam.setPosition(testPos).getPosition();
  if (pos != testPos)
  {
    std::cerr << "set/get position is not behaving as expected: " << pos[0] << "," << pos[1] << ","
              << pos[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test focal point
  f3d::camera::vector3_t testFoc = { 0., 0., -1. };
  f3d::camera::vector3_t foc = cam.setFocalPoint(testFoc).getFocalPoint();
  if (foc != testFoc)
  {
    std::cerr << "set/get focal point is not behaving as expected: " << foc[0] << "," << foc[1]
              << "," << foc[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test view up
  f3d::camera::vector3_t testUp = { 1., 0., 0. };
  f3d::camera::vector3_t up = cam.setViewUp(testUp).getViewUp();
  if (up != testUp)
  {
    std::cerr << "set/get view up is not behaving as expected: " << up[0] << "," << up[1] << ","
              << up[2] << std::endl;
    return EXIT_FAILURE;
  }

  // Test view angle
  double testAngle = 20;
  double angle = cam.setViewAngle(testAngle).getViewAngle();
  if (angle != testAngle)
  {
    std::cerr << "set/get view angle is not behaving as expected: " << angle << std::endl;
    return EXIT_FAILURE;
  }

  // Test azimuth
  cam.azimuth(90);
  f3d::camera::vector3_t expectedPos = { 0., -11., -1. };
  f3d::camera::vector3_t expectedFoc = { 1., 0., 0. };
  f3d::camera::vector3_t expectedUp = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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
  expectedFoc = { 0., 0., -1. };
  pos = cam.getPosition();
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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
  expectedUp = { 11., -11., -1. };
  pos = cam.getPosition();
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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

  // Test elevation
  cam.elevation(90);
  expectedPos = { 11., -11., -12. };
  expectedFoc = { 1., 0., 0. };
  pos = cam.getPosition();
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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
  expectedFoc = { 0., 0., -1. };
  expectedUp = { 22., -11., -12. };
  pos = cam.getPosition();
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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
  foc = cam.getViewUp();
  up = cam.getFocalPoint();
  if (!compareVec(pos, expectedPos) || !compareVec(foc, expectedFoc) || !compareVec(up, expectedUp))
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

  // Test ViewMatrix
  f3d::camera::matrix4_t mat = cam.getViewMatrix();
  f3d::camera::matrix4_t expectedMat = { 0., 0., -1., 0., 1., 0., 0., 0., 0., -1., 0., 0., 11.,
    -12., 20.9, 1. };

  if (!compareMat(mat, expectedMat))
  {
    std::cerr << "getViewMatrix is not behaving as expected: " << std::endl;
    for (int i = 0; i < 4; i++)
    {
      std::cerr << std::setprecision(12);
      for (int j = 0; j < 4; j++)
      {
        std::cerr << mat[i * 4 + j] << " ";
      }
      std::cerr << std::endl;
    }
    return EXIT_FAILURE;
  }

  f3d::camera::matrix4_t setMat = { 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., -1., 0., 0.776126,
    -0.438658, 24.556, 1. };
  cam.setViewMatrix(setMat);
  mat = cam.getViewMatrix();
  expectedMat = { 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., -0.776126, 0.438658, -24.556,
    1. };
  if (!compareMat(mat, expectedMat))
  {
    std::cerr << "setViewMatrix is not behaving as expected: " << std::endl;
    for (int i = 0; i < 4; i++)
    {
      std::cerr << std::setprecision(12);
      for (int j = 0; j < 4; j++)
      {
        std::cerr << mat[i * 4 + j] << " ";
      }
      std::cerr << std::endl;
    }
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
