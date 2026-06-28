#include "PseudoUnitTest.h"

#include <options.h>

int TestSDKOptionsDomains([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::options opt;

  // Test hasDomain
  test("hasDomain range", opt.hasDomain("scene.animation.speed_factor"));
  test("hasDomain enum", opt.hasDomain("render.effect.blending.mode"));
  test("hasDomain index", opt.hasDomain("scene.camera.index"));
  test("hasDomain false", !opt.hasDomain("model.scivis.cells"));
  test.expect<f3d::options::inexistent_exception>(
    "hasDomain inexistent", [&]() { std::ignore = opt.hasDomain("inexistent"); });

  // Test getDomainStyle
  test("getDomainStyle range",
    opt.getDomainStyle("scene.animation.speed_factor") == f3d::options::domain_style::RANGE);
  test("getDomainStyle enum",
    opt.getDomainStyle("render.effect.blending.mode") == f3d::options::domain_style::ENUM);
  test("getDomainStyle index",
    opt.getDomainStyle("scene.camera.index") == f3d::options::domain_style::INDEX);
  test.expect<f3d::options::incompatible_exception>("getDomainStyle incompatible",
    [&]() { std::ignore = opt.getDomainStyle("model.scivis.cells"); });

  // Test getEnumDomain
  test("getEnumDomain", opt.getEnumDomain("render.effect.blending.mode"),
    { "none", "ddp", "sort", "sort_cpu", "stochastic" });
  test.expect<f3d::options::incompatible_exception>(
    "getEnumDomain incompatible", [&]() { std::ignore = opt.getEnumDomain("model.scivis.cells"); });
  test.expect<f3d::options::inexistent_exception>(
    "getEnumDomain inexistent", [&]() { std::ignore = opt.getEnumDomain("inexistent"); });

  // Test increase/decrease

  opt.increase("scene.animation.speed_factor")
    .increase("scene.animation.speed_factor")
    .decrease("scene.animation.speed_factor");
  test("increase/decrease", opt.scene.animation.speed_factor, f3d::ratio_t(1.1));

  opt.scene.animation.speed_factor = f3d::ratio_t(1.9);
  opt.increase("scene.animation.speed_factor").increase("scene.animation.speed_factor");
  test("increase max cap", opt.scene.animation.speed_factor, f3d::ratio_t(2.0));

  opt.scene.animation.speed_factor = f3d::ratio_t(0.1);
  opt.decrease("scene.animation.speed_factor").decrease("scene.animation.speed_factor");
  test("decrease min cap", opt.scene.animation.speed_factor, f3d::ratio_t(0.0));

  test.expect<f3d::options::incompatible_exception>(
    "increase incompatible", [&]() { opt.increase("render.effect.blending.mode"); });
  test.expect<f3d::options::inexistent_exception>(
    "increase inexistent", [&]() { opt.increase("inexistent"); });

  // Test increase/decrease int

  opt.increase("render.raytracing.samples")
    .increase("render.raytracing.samples")
    .decrease("render.raytracing.samples");
  test("increase/decrease int", opt.render.raytracing.samples, 6);

  opt.render.raytracing.samples = 19;
  opt.increase("render.raytracing.samples").increase("render.raytracing.samples");
  test("increase max cap int", opt.render.raytracing.samples, 20);

  opt.render.raytracing.samples = 2;
  opt.decrease("render.raytracing.samples").decrease("render.raytracing.samples");
  test("decrease min cap int", opt.render.raytracing.samples, 1);

  // Test increase/decrease optional

  opt.increase("render.line_width").increase("render.line_width");
  test("increase optional", opt.render.line_width.value(), 0.1);

  opt.render.line_width = std::nullopt;
  opt.decrease("render.line_width").decrease("render.line_width");
  test("decrease optional", opt.render.line_width.value(), 9.9);

  // Test increase/decrease index

  opt.increase("scene.camera.index");
  test("increase index nullopt domain nullopt value", !opt.scene.camera.index.has_value());

  opt.domains.scene.camera.index.max = 2;
  opt.increase("scene.camera.index");
  test("increase index nullopt value", opt.scene.camera.index.value(), 0);
  opt.increase("scene.camera.index");
  test("increase index", opt.scene.camera.index.value(), 1);
  opt.increase("scene.camera.index");
  opt.increase("scene.camera.index");
  test("increase index max cap", opt.scene.camera.index.value(), 2);

  opt.scene.camera.index = std::nullopt;
  opt.decrease("scene.camera.index");
  test("decrease index nullopt value", opt.scene.camera.index.value(), 2);
  opt.decrease("scene.camera.index");
  test("decrease index", opt.scene.camera.index.value(), 1);
  opt.decrease("scene.camera.index");
  opt.decrease("scene.camera.index");
  test("decrease index min cap", opt.scene.camera.index.value(), 0);

  opt.domains.scene.camera.index.max = std::nullopt;
  opt.increase("scene.camera.index");
  test("increase index nullopt domain", opt.scene.camera.index.value(), 0);
  opt.scene.camera.index = std::nullopt;

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

  test.expect<f3d::options::incompatible_exception>(
    "cycle incompatible", [&]() { opt.cycle("scene.animation.speed_factor"); });
  test.expect<f3d::options::inexistent_exception>(
    "cycle inexistent", [&]() { opt.cycle("inexistent"); });

  // Test cycle index

  opt.cycle("scene.camera.index");
  test("cycle index nullopt domain nullopt value", !opt.scene.camera.index.has_value());

  opt.domains.scene.camera.index.max = 2;
  opt.cycle("scene.camera.index");
  test("cycle index nullopt", opt.scene.camera.index.value(), 0);
  opt.cycle("scene.camera.index");
  test("cycle index", opt.scene.camera.index.value(), 1);
  opt.cycle("scene.camera.index");
  opt.cycle("scene.camera.index");
  test("cycle index loop nullopt", !opt.scene.camera.index.has_value());
  opt.cycle("scene.camera.index");
  test("cycle index loop", opt.scene.camera.index.value(), 0);
  opt.scene.camera.index = 3;
  opt.cycle("scene.camera.index");
  test("cycle index loop invalid", opt.scene.camera.index.value(), 0);

  opt.domains.scene.camera.index.max = std::nullopt;
  opt.cycle("scene.camera.index");
  test("cycle index nullopt domain", opt.scene.camera.index.value(), 0);

  // Check individual domains

  // clang-format off
  // render.effect.blending.mode: tested above
  // scene.animation.speed_factor: tested above
  // render.raytracing.samples: tested above

  test("model.color.opacity range", opt.domains.model.color.opacity.range, {0, 1});
  test("model.color.opacity increment", opt.domains.model.color.opacity.increment, 0.05);

  test("scene.animation.speed_factor range", opt.domains.scene.animation.speed_factor.range, {f3d::ratio_t(0), f3d::ratio_t(2)});
  test("scene.animation.speed_factor increment", opt.domains.scene.animation.speed_factor.increment, f3d::ratio_t(0.1));

  test("model.material.base_ior range", opt.domains.model.material.base_ior.range, {0, 1});
  test("model.material.base_ior increment", opt.domains.model.material.base_ior.increment, 0.05);

  test("model.material.metallic range", opt.domains.model.material.metallic.range, {0, 1});
  test("model.material.metallic increment", opt.domains.model.material.metallic.increment, 0.05);

  test("model.material.roughness range", opt.domains.model.material.roughness.range, {0, 1});
  test("model.material.roughness increment", opt.domains.model.material.roughness.increment, 0.05);

  test("model.normal.scale range", opt.domains.model.normal.scale.range, {0, 1});
  test("model.normal.scale increment", opt.domains.model.normal.scale.increment, 0.05);

  test("model.normal_glyphs.scale range", opt.domains.model.normal_glyphs.scale.range, {f3d::ratio_t(0), f3d::ratio_t(10)});
  test("model.normal_glyphs.scale increment", opt.domains.model.normal_glyphs.scale.increment, f3d::ratio_t(0.1));

  test("model.scivis.discretization range", opt.domains.model.scivis.discretization.range, {1, 1024});
  test("model.scivis.discretization increment", opt.domains.model.scivis.discretization.increment, 5);

  test("render.background.blur.coc range", opt.domains.render.background.blur.coc.range, {0., 100.});
  test("render.background.blur.coc increment", opt.domains.render.background.blur.coc.increment, 5.);

  test("render.light.intensity range", opt.domains.render.light.intensity.range, {0, 5});
  test("render.light.intensity increment", opt.domains.render.light.intensity.increment, 0.02);

  test("render.line_width range", opt.domains.render.line_width.range, {0, 10});
  test("render.line_width increment", opt.domains.render.line_width.increment, 0.1);

  test("render.point_size range", opt.domains.render.point_size.range, {0, 10});
  test("render.point_size increment", opt.domains.render.point_size.increment, 0.1);

  test("ui.backdrop.opacity range", opt.domains.ui.backdrop.opacity.range, {0, 1});
  test("ui.backdrop.opacity increment", opt.domains.ui.backdrop.opacity.increment, 0.05);

  test("ui.scale range", opt.domains.ui.scale.range, {f3d::ratio_t(0), f3d::ratio_t(10)});
  test("ui.scale increment", opt.domains.ui.scale.increment, f3d::ratio_t(0.1));

  test("interactor.style enum", opt.getEnumDomain("interactor.style"), {"default", "trackball", "2d"});
  test("model.point_sprites.type enum", opt.getEnumDomain("model.point_sprites.type"), {"none", "sphere", "gaussian", "circle", "stddev", "bound","cross"});
  test("render.backface_type enum", opt.getEnumDomain("render.backface_type"), {"visible", "hidden"});
  test("render.effect.antialiasing.mode enum", opt.getEnumDomain("render.effect.antialiasing.mode"), {"none", "fxaa", "ssaa", "taa"});

  test("scene.camera.index max index", !opt.domains.scene.camera.index.max.has_value());
  // clang-format on

  return test.result();
}
