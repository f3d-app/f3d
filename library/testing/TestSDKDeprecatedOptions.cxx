#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

int TestSDKDeprecatedOptions([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  opt.render.effect.anti_aliasing = true;
  opt.render.effect.translucency_support = true;

  sce.add(std::string(argv[1]) + "/data/cow.vtp");

  win.render();

  test("Deprecated options and render", TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDeprecatedOptions"));
  return test.result();
}

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
