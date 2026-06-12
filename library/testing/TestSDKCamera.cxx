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
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

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

  // Test position, focal point, up, and angle
  test("set/get position", cam.setPosition({ 0., 0., 10. }).getPosition(), { 0., 0., 10. });
  test("set/get focal point", cam.setFocalPoint({ 0., 0., -1. }).getFocalPoint(), { 0., 0., -1. });
  test("set/get view up", cam.setViewUp({ 1., 0., 0. }).getViewUp(), { 1., 0., 0. });
  test("set/get view angle", cam.setViewAngle(20).getViewAngle(), 20.0);

  // Test movement methods
  cam.setPosition({ 0, 0, 10 }).setFocalPoint({ 0, 0, 0 }).setViewUp({ 0, 1, 0 });
  cam.azimuth(90);
  test("azimuth method", cam.getPosition(), approx(f3d::point3_t{ -10., 0., 0. }));

  cam.setPosition({ 0, 0, 10 }).setFocalPoint({ 0, 0, 0 }).setViewUp({ 0, 1, 0 });
  cam.elevation(90);
  test("elevation method", cam.getPosition(), approx(f3d::point3_t{ 0., -10., 0. }));

  // -------------------------------------------------------------------------
  // Getter Equivalence: Using a fixed camera frame to ensure coordinate stability
  // -------------------------------------------------------------------------

  const std::vector<f3d::direction_t> up_directions = { { 0, 0, 1 }, { 0, 1, 0 }, { 1, 0, 0 } };
  const std::vector<std::pair<double, double>> az_el = { { 0, 0 }, { 12, 34 }, { -12, -34 } };

  for (const auto up : up_directions)
  {
    for (const auto [a, e] : az_el)
    {
      // Reset to a known state for every iteration
      cam.setPosition({ 0, 0, 10 }).setFocalPoint({ 0, 0, 0 }).setViewUp(up);
      cam.azimuth(a).elevation(e);

      const std::string title =
        " (up=" + f3d::options::format(up) + ", a=" + std::to_string((int)a) + ")";
      test("get azimuth" + title, cam.getWorldAzimuth(), approx(a, 1e-5));
      test("get elevation" + title, cam.getWorldElevation(), approx(e, 1e-5));
    }
  }

  return test.result();
}