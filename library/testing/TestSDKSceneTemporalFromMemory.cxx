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

  std::string texturePath = std::string(argv[1]) + "data/world.png";
  eng.getOptions().model.color.texture = texturePath;
  const std::string outputPath = std::string(argv[2]);

  const std::vector<float> basePoints{ 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f };
  const std::vector<float> translatedPoints{ 0.25f, 0.f, 0.f, 0.25f, 1.f, 0.f, 1.25f, 0.f, 0.f,
    1.25f, 1.f, 0.f };
  const std::vector<float> normals{ 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f,
    -1.f };
  const std::vector<float> tcoords{ 0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f };
  const std::vector<unsigned int> faceSides{ 3, 3 };
  const std::vector<unsigned int> faceIndices{ 0, 1, 2, 1, 3, 2 };

  f3d::mesh_t meshFrame0{ basePoints, normals, tcoords, faceSides, faceIndices };
  f3d::mesh_t meshFrame1{ translatedPoints, normals, tcoords, faceSides, faceIndices };

  test("add temporal mesh sequence",
    [&]() { sce.add(std::vector<f3d::mesh_t>{ meshFrame0, meshFrame1 }, 1.0); });

  test("animation time range", [&]() {
    auto range = sce.animationTimeRange();
    return std::abs(range.first - 0.0) < 1e-6 && std::abs(range.second - 1.0) < 1e-6;
  });

  const std::string baselinePath = std::string(argv[1]) + "baselines/TestSDKSceneFromMemory.png";
  test("qualitative baseline comparison", [&]() {
    sce.loadAnimationTime(0.0);
    f3d::image frame0 = win.renderToImage();
    frame0.save(outputPath + "TestSDKSceneTemporalFromMemory_frame0.png");
    f3d::image baseline(baselinePath);
    double error = frame0.compare(baseline);
    if (error > 0.05)
    {
      std::ostringstream stream;
      stream << "baseline difference is " << error;
      throw stream.str();
    }
  });

  test("quantitative temporal difference", [&]() {
    sce.loadAnimationTime(0.0);
    f3d::image frame0 = win.renderToImage();
    frame0.save(outputPath + "TestSDKSceneTemporalFromMemory_frame0.png");
    sce.loadAnimationTime(1.0);
    f3d::image frame1 = win.renderToImage();
    frame1.save(outputPath + "TestSDKSceneTemporalFromMemory_frame1.png");
    double error = frame0.compare(frame1);
    if (error <= 0.01)
    {
      std::ostringstream stream;
      stream << "frame delta is " << error;
      throw stream.str();
    }
  });

  return test.result();
}
