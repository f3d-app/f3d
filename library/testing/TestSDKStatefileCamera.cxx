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
  src.saveStatefile(statefilePath);

  // Restore into another windowed engine and check the camera is restored
  f3d::engine dst = f3d::engine::create(true);
  dst.loadStatefile(statefilePath);
  const f3d::camera_state_t restored = dst.getWindow().getCamera().getState();
  test("restored camera position", restored.position, approx(saved.position));
  test("restored camera focal point", restored.focalPoint, approx(saved.focalPoint));
  test("restored camera view up", restored.viewUp, approx(saved.viewUp));
  test("restored camera view angle", restored.viewAngle, approx(saved.viewAngle));

  return test.result();
}
