#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <log.h>
#include <scene.h>
#include <window.h>

namespace fs = std::filesystem;

int TestSDKSceneLights([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  const std::string renderingBackend = argv[4];
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow().setSize(300, 300);
  f3d::options& options = eng.getOptions();

  options.render.grid.enable = true;
  options.render.shadows.enable = true;

  f3d::light_state_t topFrontBlueLight = {
    .type = f3d::light_type::SCENE_LIGHT,
    .position = f3d::point3_t{0.0, 0.0, 0.0},
    .color = f3d::color_t{0.0, 0.0, 1.0},
    .direction = f3d::vector3_t{0.0, -1.0, -1.0},
    .positionalLight = false,
    .intensity = 1.0,
    .switchState = true,
    .castShadows = true
  };

  f3d::light_state_t leftRightLight = {
    .type = f3d::light_type::SCENE_LIGHT,
    .position = f3d::point3_t{-9.0, 0.0, 0.0},
    .color = f3d::color_t{1.0, 0.0, 0.0},
    .direction = f3d::vector3_t{1.0, 0.0, 0.0},
    .positionalLight = true,
    .intensity = 1.0,
    .switchState = true,
    .castShadows = false
  };

  f3d::light_state_t rightDisabledLight = {
    .type = f3d::light_type::SCENE_LIGHT,
    .position = f3d::point3_t{9.0, 0.0, 1.0},
    .color = f3d::color_t{1.0, 1.0, 1.0},
    .direction = f3d::vector3_t{-1.0, 0.0, 0.0},
    .positionalLight = true,
    .intensity = 1.0,
    .switchState = false,
    .castShadows = false
  };

  sce.addLight(topFrontBlueLight);
  sce.addLight(leftRightLight);
  sce.addLight(rightDisabledLight);

  sce.add(std::string(argv[1]) + "data/cow.vtp");

  test("render",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneLights"));

  return test.result();
}
