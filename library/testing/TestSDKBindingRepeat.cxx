#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>
#include <window.h>

#include <functional>
#include <string>

using mod_t = f3d::interaction_bind_t::ModifierKeys;

int TestSDKBindingRepeat([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  const std::string dataPath = std::format("{}data/cow.vtp", std::string(argv[1]));
  const std::string baselinePath = std::format("{}baselines/", std::string(argv[1]));
  const std::string outputPath = std::string(argv[2]);
  const std::string renderingBackend = std::string(argv[4]);

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::window& win = eng.getWindow();
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();

  win.setSize(300, 300);
  sce.add(dataPath);
  win.render();

  inter.removeBinding({ mod_t::CTRL, "P" });
  inter.addBinding({ mod_t::CTRL, "P" }, "increase model.color.opacity", {}, nullptr,
    f3d::interactor::BindingType::NUMERICAL, true, false);

  std::string filename = "TestSDKBindingRepeatOff";
  std::string interactionFilePath = std::format("{}../recordings/{}.log", baselinePath, filename);
  test("play binding repeat off interaction", inter.playInteraction(interactionFilePath));

  test("trigger binding no repeat",
    TestSDKHelpers::RenderTest(win, baselinePath, outputPath, "TestSDKBindingRepeatOff"));

  inter.removeBinding({ mod_t::CTRL, "P" });
  inter.addBinding({ mod_t::CTRL, "P" }, "increase model.color.opacity", {}, nullptr,
    f3d::interactor::BindingType::NUMERICAL, true, true);

  // Uses a different file to add extra interaction as the render state will already have the axis
  // on
  filename = "TestSDKBindingRepeatOn";
  interactionFilePath = std::format("{}../recordings/{}.log", baselinePath, filename);
  test("play binding repeat interaction", inter.playInteraction(interactionFilePath));

  test("trigger binding repeat",
    TestSDKHelpers::RenderTest(win, baselinePath, outputPath, "TestSDKBindingRepeatOn"));

  return test.result();
}
