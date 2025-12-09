#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

#include <iostream>

int TestSDKTriggerInteractionsWithoutRender([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  std::string DataPath = std::string(argv[1]) + "/data/cow.vtp";
  eng.getWindow().setSize(300, 300);
  eng.getScene().add(DataPath);
  f3d::interactor& inter = eng.getInteractor();
  
  // Trigger mouse wheel backward and check if it zoomed out
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::BACKWARD);
  
  // Trigger mouse wheel forward and check if it zoomed in
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::FORWARD);
  
  // Trigger edge rendering with key press "E"
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");

  // Trigger decreased light rendering with key press "SHIFT+L"
  inter.triggerModUpdate(f3d::interactor::InputModifier::SHIFT);
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "L");

  // Trigger character inputs
  
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "Escape"); // show console
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Escape");
  inter.triggerTextCharacter('f');
  inter.triggerTextCharacter('3');
  inter.triggerTextCharacter('d');
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "Return");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "Return");

  // Trigger rotate camera with mouse left button
  inter.triggerMousePosition(150, 100);
  inter.triggerMouseButton(f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::LEFT);
  inter.triggerMousePosition(150, 200);
  inter.triggerMouseButton(f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::LEFT);
  
  // Trigger pan camera with mouse middle button
  inter.triggerMousePosition(150, 100);
  inter.triggerMouseButton(
      f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::MIDDLE);
  inter.triggerMousePosition(150, 200);
  inter.triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::MIDDLE);

  // Trigger zoom camera with mouse right button
  inter.triggerMousePosition(150, 100);
  inter.triggerMouseButton(
      f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::RIGHT);
  inter.triggerMousePosition(150, 200);
  inter.triggerMouseButton(
      f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::RIGHT);

  // following calls are only there for coverage
  // there is no interactor style action associated with these yet
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::LEFT);
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::RIGHT);

  return EXIT_SUCCESS;
}
