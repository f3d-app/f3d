#include "PseudoUnitTest.h"

#include <camera.h>
#include <engine.h>
#include <window.h>

#include <filesystem>

namespace fs = std::filesystem;

int TestSDKStatefileCamera([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  const fs::path statefilePath = fs::path(argv[2]) / "camera_statefile.json";

  // Save a statefile from an engine with a window so the camera state is captured
  f3d::engine src = f3d::engine::create(true);
  const f3d::camera_state_t state{ { 1., 2., 3. }, { 0., 0., 0. }, { 0., 1., 0. }, 25. };
  src.getWindow().getCamera().setState(state);

  // Compare against the state as the camera actually holds it (the view up is reorthogonalized)
  const f3d::camera_state_t saved = src.getWindow().getCamera().getState();

  // Also capture a window size, compared against the size the window actually holds
  src.getWindow().setSize(512, 384);
  const int savedWidth = src.getWindow().getWidth();
  const int savedHeight = src.getWindow().getHeight();

  // Also capture a window position
  src.getWindow().setPosition(64, 96);

  src.dump().toFile(statefilePath);

  // Restore into another windowed engine and check the camera is restored
  f3d::engine dst = f3d::engine::create(true);
  dst.load(f3d::engine::state::fromFile(statefilePath));
  const f3d::camera_state_t restored = dst.getWindow().getCamera().getState();
  test("restored camera position", restored.position, approx(saved.position));
  test("restored camera focal point", restored.focalPoint, approx(saved.focalPoint));
  test("restored camera view up", restored.viewUp, approx(saved.viewUp));
  test("restored camera view angle", restored.viewAngle, approx(saved.viewAngle));
  test("restored window width", dst.getWindow().getWidth(), savedWidth);
  test("restored window height", dst.getWindow().getHeight(), savedHeight);
  test("restored window position x", dst.getWindow().getPositionX(), 64);
  test("restored window position y", dst.getWindow().getPositionY(), 96);

  // A legacy statefile without a window position is still valid: size is restored and the position
  // is left untouched
  f3d::engine legacy = f3d::engine::create(true);
  legacy.load(f3d::engine::state::fromString(R"({ "window": { "width": 256, "height": 128 } })"));
  test("restored legacy window width", legacy.getWindow().getWidth(), 256);
  test("restored legacy window height", legacy.getWindow().getHeight(), 128);

  return test.result();
}
