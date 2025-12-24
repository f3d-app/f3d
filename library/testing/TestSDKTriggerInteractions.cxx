#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

#include <iostream>

struct TestTriggerHelper
{
  TestTriggerHelper(std::string dataPath, std::string baselinePath, std::string outputPath)
    : DataPath(std::move(dataPath))
    , BaselinePath(std::move(baselinePath))
    , OutputPath(std::move(outputPath))
  {
  }

  template<typename Func>
  void operator()(Func&& func) const
  {
    f3d::engine engine = f3d::engine::create();
    engine.getWindow().setSize(300, 300);
    engine.getScene().add(this->DataPath);

    func(engine);
  }

  // Run with baseline test
  template<typename Func>
  void operator()(const std::string& name, Func&& func) const
  {
    (*this)([&](f3d::engine& engine) {
      func(engine);

      if (!TestSDKHelpers::RenderTest(
            engine.getWindow(), this->BaselinePath, this->OutputPath, name))
      {
        throw std::runtime_error("Render test failed for " + name);
      }
    });
  }

  std::string DataPath;
  std::string BaselinePath;
  std::string OutputPath;
};

int TestSDKTriggerInteractions([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  TestTriggerHelper helper(
    std::string(argv[1]) + "/data/cow.vtp", std::string(argv[1]) + "baselines/", argv[2]);

  // Trigger mouse wheel backward and check if it zoomed out
  helper("TestSDKTriggerInteractionsWheelBackward", [](f3d::engine& engine) {
    engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::BACKWARD);
  });

  // Trigger mouse wheel forward and check if it zoomed in
  helper("TestSDKTriggerInteractionsWheelForward", [](f3d::engine& engine) {
    engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::FORWARD);
  });

  // Trigger edge rendering with key press "E"
  helper("TestSDKTriggerInteractionsKeyPress", [](f3d::engine& engine) {
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");
  });

  // Trigger decreased light rendering with key press "SHIFT+L"
  helper("TestSDKTriggerInteractionsKeyPressMod", [](f3d::engine& engine) {
    for (int i = 0; i < 30; ++i)
    {
      engine.getInteractor().triggerModUpdate(f3d::interactor::InputModifier::SHIFT);
      engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "L");
    }
  });

  // Trigger character inputs
  helper("TestSDKTriggerInteractionsTextInput", [](f3d::engine& engine) {
    engine.getInteractor().triggerKeyboardKey(
      f3d::interactor::InputAction::PRESS, "Escape"); // show console
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Escape");
    engine.getInteractor().triggerTextCharacter('f');
    engine.getInteractor().triggerTextCharacter('3');
    engine.getInteractor().triggerTextCharacter('d');
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "Return");
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Return");
  });

  // Trigger rotate camera with mouse left button
  helper("TestSDKTriggerInteractionsMouseRotate", [](f3d::engine& engine) {
    engine.getInteractor().triggerMousePosition(150, 100);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::LEFT);
    engine.getInteractor().triggerMousePosition(150, 200);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::LEFT);
  });

  // Trigger pan camera with mouse middle button
  helper("TestSDKTriggerInteractionsMousePan", [](f3d::engine& engine) {
    engine.getInteractor().triggerMousePosition(150, 100);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::MIDDLE);
    engine.getInteractor().triggerMousePosition(150, 200);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::MIDDLE);
  });

  // Trigger zoom camera with mouse right button
  helper("TestSDKTriggerInteractionsMouseZoom", [](f3d::engine& engine) {
    engine.getInteractor().triggerMousePosition(150, 100);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::RIGHT);
    engine.getInteractor().triggerMousePosition(150, 200);
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::RIGHT);
  });

  // following calls are only there for coverage
  // there is no interactor style action associated with these yet
  helper([](f3d::engine& engine) {
    engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::LEFT);
    engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::RIGHT);
  });

  helper([](f3d::engine& engine) {
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::LEFT);
  });

  helper([](f3d::engine& engine) {
    engine.getInteractor().triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::RIGHT);
  });

  helper([](f3d::engine& engine) {
    engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Return");
  });

  helper([](f3d::engine& engine) { engine.getInteractor().triggerTextCharacter('f'); });

  return EXIT_SUCCESS;
}
