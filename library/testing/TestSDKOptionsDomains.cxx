#include "PseudoUnitTest.h"

#include <options.h>

int TestSDKOptionsDomains([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::options opt;

  // Test hasDomain
  f3d::options::domain_style style;
  test("hasDomain range", opt.hasDomain("render.light.intensity", style));
  test("hasDomain check range", style == f3d::options::domain_style::RANGE);
  test("hasDomain enum", opt.hasDomain("render.effect.blending.mode", style));
  test("hasDomain check enum", style == f3d::options::domain_style::ENUM);
  test("hasDomain false", !opt.hasDomain("model.scivis.cells", style));
  test.expect<f3d::options::inexistent_exception>(
    "hasDomain inexistent", [&]() { std::ignore = opt.hasDomain("inexistent", style); });

  // Test getDomain
  test("getDomain range", opt.getDomain("render.light.intensity"), { "0", "5", "0.02" });
  test("getDomain enum", opt.getDomain("render.effect.blending.mode"),
    { "none", "ddp", "sort", "sort_cpu", "stochastic" });
  test.expect<f3d::options::incompatible_exception>(
    "getDomain incompatible", [&]() { std::ignore = opt.getDomain("model.scivis.cells"); });
  test.expect<f3d::options::inexistent_exception>(
    "getDomain inexistent", [&]() { std::ignore = opt.getDomain("inexistent"); });

  // Test increase/decrease

  opt.increase("render.light.intensity")
    .increase("render.light.intensity")
    .decrease("render.light.intensity");
  test("increase/decrease", opt.render.light.intensity, 1.02);

  opt.render.light.intensity = 4.98;
  opt.increase("render.light.intensity").increase("render.light.intensity");
  test("increase max cap", opt.render.light.intensity, 5.0);

  opt.render.light.intensity = 0.02;
  opt.decrease("render.light.intensity").decrease("render.light.intensity");
  test("decrease min cap", opt.render.light.intensity, 0.0);

  test.expect<f3d::options::incompatible_exception>(
    "increase incompatible", [&]() { opt.increase("render.effect.blending.mode"); });
  test.expect<f3d::options::inexistent_exception>(
    "increase inexistent", [&]() { opt.increase("inexistent"); });

  // Test increase/decrease int

  opt.increase("render.raytracing.samples")
    .increase("render.light.intensity")
    .decrease("render.light.intensity");
  test("increase/decrease int", opt.render.raytracing.samples, 6);

  opt.render.raytracing.samples = 19;
  opt.increase("render.raytracing.samples").increase("render.light.intensity");
  test("increase max cap int", opt.render.raytracing.samples, 20);

  opt.render.raytracing.samples = 2;
  opt.decrease("render.raytracing.samples").decrease("render.light.intensity");
  test("decrease min cap int", opt.render.raytracing.samples, 1);

  // Test increase/decrease optional

  opt.increase("render.line_width").increase("render.line_width");
  test("increase optional", opt.render.line_width.value(), 0.1);

  opt.render.line_width = std::nullopt;
  opt.decrease("render.line_width").decrease("render.line_width");
  test("decrease optional", opt.render.line_width.value(), 9.9);

  // Test cycle

  opt.cycle("render.effect.blending.mode");
  test("cycle", opt.render.effect.blending.mode == "ddp");

  opt.cycle("render.effect.blending.mode")
    .cycle("render.effect.blending.mode")
    .cycle("render.effect.blending.mode")
    .cycle("render.effect.blending.mode");
  test("cycle loop", opt.render.effect.blending.mode == "none");

  opt.render.effect.blending.mode = "invalid";
  opt.cycle("render.effect.blending.mode");
  test("cycle from invalid", opt.render.effect.blending.mode == "none");

  opt.cycle("render.backface_type");
  test("cycle optional", opt.render.backface_type.value() == "visible");

  opt.cycle("render.backface_type").cycle("render.backface_type");
  test("cycle optional reset", !opt.render.backface_type.has_value());

  opt.cycle("scene.camera.index");
  test("cycle empty domain optional", !opt.scene.camera.index.has_value());

  opt.scene.camera.index = 0;
  opt.cycle("scene.camera.index");
  test("cycle empty domain", opt.scene.camera.index.value(), 0);

  test.expect<f3d::options::incompatible_exception>(
    "cycle incompatible", [&]() { opt.cycle("render.light.intensity"); });
  test.expect<f3d::options::inexistent_exception>(
    "cycle inexistent", [&]() { opt.cycle("inexistent"); });

  // Check individual domains

  // clang-format off
  // render.effect.blending.mode: tested above
  // render.light.intensity: tested above
  // render.raytracing.samples: tested above
  test("interactor.style domain", opt.getDomain("interactor.style"), {"default", "trackball", "2d"});
  test("model.color.opacity domain", opt.getDomain("model.color.opacity"), {"0", "1", "0.05"});
  test("model.material.base_ior domain", opt.getDomain("model.material.base_ior"), {"0", "1", "0.05"});
  test("model.material.metallic domain", opt.getDomain("model.material.metallic"), {"0", "1", "0.05"});
  test("model.material.roughness domain", opt.getDomain("model.material.roughness"), {"0", "1", "0.05"});
  test("model.normal.scale domain", opt.getDomain("model.normal.scale"), {"0", "1", "0.05"});
  test("model.normal_glyphs.scale domain", opt.getDomain("model.normal_glyphs.scale"), {"0", "10", "0.1"});
  test("model.point_sprites.size domain", opt.getDomain("model.point_sprites.size"), {"1", "100", "1"});
  test("model.point_sprites.type domain", opt.getDomain("model.point_sprites.type"), {"none", "sphere", "gaussian", "circle", "stddev", "bound","cross"});
  test("model.scivis.discretization domain", opt.getDomain("model.scivis.discretization"), {"1", "1024", "5"});
  test("render.backface_type domain", opt.getDomain("render.backface_type"), {"visible", "hidden"});
  test("render.background.blur.coc domain", opt.getDomain("render.background.blur.coc"), {"0", "100", "5"});
  test("render.effect.antialiasing.mode domain", opt.getDomain("render.effect.antialiasing.mode"), {"none", "fxaa", "ssaa", "taa"});
  test("render.line_width domain", opt.getDomain("render.line_width"), {"0", "10", "0.1"});
  test("render.point_size domain", opt.getDomain("render.point_size"), {"0", "10", "0.1"});
  test("scene.animation.speed_factor domain", opt.getDomain("scene.animation.speed_factor"), {"0", "2", "0.1"});
  test("scene.camera.index domain", opt.getDomain("scene.camera.index"), {});
  test("ui.backdrop.opacity domain", opt.getDomain("ui.backdrop.opacity"), {"0", "1", "0.05"});
  test("ui.scale domain", opt.getDomain("ui.scale"), {"0", "10", "0.1"});
  // clang-format on

  return test.result();
}
