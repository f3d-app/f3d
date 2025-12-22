#include "PseudoUnitTest.h"

#include <engine.h>
#include <image.h>
#include <log.h>
#include <scene.h>
#include <window.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

int TestSDKSceneTemporalFromMemory([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow().setSize(300, 300);

  // Define mesh data for two frames
  const std::vector<float> basePoints{ 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f };
  const std::vector<float> translatedPoints{ 0.25f, 0.f, 0.f, 0.25f, 1.f, 0.f, 1.25f, 0.f, 0.f,
    1.25f, 1.f, 0.f };
  const std::vector<float> normals{ 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f,
    -1.f };
  const std::vector<float> tcoords{ 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f };
  const std::vector<unsigned int> faceSides{ 3, 3 };
  const std::vector<unsigned int> faceIndices{ 0, 1, 2, 1, 3, 2 };

  auto meshCallback = [&](double time) -> f3d::mesh_t {
    if (time < 0.5)
    {
      return f3d::mesh_t{ basePoints, normals, tcoords, faceSides, faceIndices };
    }
    else
    {
      return f3d::mesh_t{ translatedPoints, normals, tcoords, faceSides, faceIndices };
    }
  };

  test("add animated mesh with callback", [&]() { sce.add({ 0.0, 1.0 }, meshCallback); });

  test("animation time range", [&]() {
    auto range = sce.animationTimeRange();
    return std::abs(range.first - 0.0) < 1e-6 && std::abs(range.second - 1.0) < 1e-6;
  });

  test("quantitative temporal difference", [&]() {
    sce.loadAnimationTime(0.0);
    f3d::image frame0 = win.renderToImage();
    sce.loadAnimationTime(1.0);
    f3d::image frame1 = win.renderToImage();
    double error = frame0.compare(frame1);
    return error > 0.01;
  });

  return test.result();
}
