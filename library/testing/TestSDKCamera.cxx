#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <camera.h>
#include <engine.h>
#include <log.h>
#include <options.h>
#include <window.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

int TestSDKCamera([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::camera& cam = win.getCamera();

  // Basic tests
  test("set/get position", cam.setPosition({ 0., 0., 10. }).getPosition(), { 0., 0., 10. });
  test("set/get focal point", cam.setFocalPoint({ 0., 0., -1. }).getFocalPoint(), { 0., 0., -1. });

  // Getter Equivalence tests
  const std::vector<f3d::direction_t> up_directions = { { 0, 0, 1 }, { 0, 1, 0 }, { 1, 0, 0 } };
  const std::vector<std::pair<double, double>> az_el = { { 0, 0 }, { 12, 34 }, { -12, -34 } };

  for (const auto& up : up_directions)
  {
    for (const auto& [a, e] : az_el)
    {
      // Reset camera state to a known identity to avoid rotational drift
      cam.setPosition({ 0, 0, 10 }).setFocalPoint({ 0, 0, 0 }).setViewUp(up);
      cam.azimuth(a).elevation(e);

      std::string title = " (up=" + f3d::options::format(up) + ", a=" + std::to_string(static_cast<int>(a)) + ")";
      
      // We use a tolerance of 0.5 degrees. This is required because VTK internally 
      // re-normalizes the camera's orthonormal basis (Right, Up, Forward) after 
      // floating-point matrix rotations.
      test("get azimuth" + title, cam.getWorldAzimuth(), approx(a, 0.5));
      test("get elevation" + title, cam.getWorldElevation(), approx(e, 0.5));
    }
  }

  return test.result();
}