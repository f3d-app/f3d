#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>
#include <window.h>

#include <functional>
#include <string>
#include <utility>

int TestSDKNotification([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  std::string DataPath = std::string(argv[1]) + "data/cow.vtp";
  std::string BaselinePath = std::string(argv[1]) + "baselines/";
  std::string OutputPath = std::string(argv[2]);

  f3d::engine eng = f3d::engine::create();
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();

  win.setSize(300, 300);
  opt.ui.feedback.enable = true;
  opt.ui.feedback.show_bindings = true;
  sce.add(DataPath);
  win.render();

  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "T");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "T");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "G");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "G");

  test("stacking notifications",
    TestSDKHelpers::RenderTest(win, BaselinePath, OutputPath, "TestSDKNotificationStack"));

  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");
  inter.triggerEventLoop(0.1);
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "T");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "T");
  inter.triggerEventLoop(0.1);
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "G");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "G");
  inter.triggerEventLoop(2.5);

  test("notifications fading",
    TestSDKHelpers::RenderTest(win, BaselinePath, OutputPath, "TestSDKNotificationFading"));
  inter.triggerEventLoop(.5);

  auto docTgl = [&](const std::string& doc, const bool& value) {
    return std::pair(doc, (value ? "ON" : "OFF"));
  };

  inter.addBinding({ f3d::interaction_bind_t::ModifierKeys::ANY, "Exclam" },
    "toggle ui.feedback.show_bindings", "Custom",
    std::bind(docTgl, "Show Bind Keys", std::cref(opt.ui.feedback.show_bindings)),
    f3d::interactor::BindingType::TOGGLE);

  inter.initBindNotificationMap();

  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "Exclam");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Exclam");
  inter.addNotification("Test Notification", "Test Value");

  test("user define notifications",
    TestSDKHelpers::RenderTest(win, BaselinePath, OutputPath, "TestSDKNotificationUser"));

  inter.triggerEventLoop(3.0);
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "T");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "T");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "G");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "G");

  test("turn off binding keys visibility",
    TestSDKHelpers::RenderTest(win, BaselinePath, OutputPath, "TestSDKNotificationKeysOff"));

  opt.ui.feedback.enable = false;

  test("turn off notification visibility",
    TestSDKHelpers::RenderTest(win, BaselinePath, OutputPath, "TestSDKNotificationOff"));

  return test.result();
}
