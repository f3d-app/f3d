#include <engine.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <iostream>
#include <string_view>

struct TestTriggerHelper
{
  TestTriggerHelper(std::string dataPath, std::string baselinePath, std::string outputPath)
    : DataPath(std::move(dataPath))
    , BaselinePath(std::move(baselinePath))
    , OutputPath(std::move(outputPath))
  {
  }

  template<typename Func>
  void operator()(const std::string& name, Func&& func) const
  {
    f3d::engine eng = f3d::engine::create();
    eng.getWindow().setSize(300, 300);
    eng.getScene().add(this->DataPath);
    eng.getWindow().render();

    func(eng);

    if (!TestSDKHelpers::RenderTest(eng.getWindow(), this->BaselinePath, this->OutputPath, name))
    {
      throw std::runtime_error("Render test failed for " + name);
    }
  }

  std::string DataPath;
  std::string BaselinePath;
  std::string OutputPath;
};

int TestSDKTriggerInteractions(int argc, char* argv[])
{
  TestTriggerHelper helper(
    std::string(argv[1]) + "/data/cow.vtp", std::string(argv[1]) + "baselines/", argv[2]);

  try
  {
    // Trigger mouse wheel backward and check if it zoomed out
    helper("TestSDKTriggerInteractionsWheelBackward", [](f3d::engine& engine) {
      engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::BACKWARD);

      // following calls are only there for coverage
      // there is no interactor style action associated with these
      engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::LEFT);
      engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::RIGHT);
    });

    // Trigger mouse wheel forward and check if it zoomed in
    helper("TestSDKTriggerInteractionsWheelForward", [](f3d::engine& engine) {
      engine.getInteractor().triggerMouseWheel(f3d::interactor::WheelDirection::FORWARD);
    });

    // Trigger edge rendering with key press "E"
    helper("TestSDKTriggerInteractionsKeyPress", [](f3d::engine& engine) {
      engine.getInteractor().triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");

      // following call is only there for coverage
      // there is no interactor style action associated with this
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

    // Trigger rotate camera with mouse middle button
    helper("TestSDKTriggerInteractionsMousePan", [](f3d::engine& engine) {
      engine.getInteractor().triggerMousePosition(150, 100);
      engine.getInteractor().triggerMouseButton(
        f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::MIDDLE);
      engine.getInteractor().triggerMousePosition(150, 200);
      engine.getInteractor().triggerMouseButton(
        f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::MIDDLE);
    });

    // Trigger rotate camera with mouse right button
    helper("TestSDKTriggerInteractionsMouseZoom", [](f3d::engine& engine) {
      engine.getInteractor().triggerMousePosition(150, 100);
      engine.getInteractor().triggerMouseButton(
        f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::RIGHT);
      engine.getInteractor().triggerMousePosition(150, 200);
      engine.getInteractor().triggerMouseButton(
        f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::RIGHT);
    });
  }
  catch (const std::exception& e)
  {
    std::cerr << "An exception has been thrown: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
