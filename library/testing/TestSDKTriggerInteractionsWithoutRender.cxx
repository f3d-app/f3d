#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

#include <iostream>

int TestSDKTriggerInteractionsWithoutRender(
  [[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  std::string DataPath = std::string(argv[1]) + "/data/cow.vtp";
  eng.getWindow().setSize(300, 300);
  eng.getScene().add(DataPath);
  f3d::interactor& inter = eng.getInteractor();

  // Trigger mouse wheel
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::BACKWARD);
  inter.triggerMouseWheel(f3d::interactor::WheelDirection::FORWARD);

  // Trigger keyboard key
  inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, "E");
  inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, "E");

  // Trigger modifier update
  inter.triggerModUpdate(f3d::interactor::InputModifier::SHIFT);

  // Trigger character inputs
  inter.triggerTextCharacter('f');

  // Trigger rotate camera with mouse left button
  inter.triggerMousePosition(150, 100);

  // Trigger mouse button
  inter.triggerMouseButton(f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::LEFT);
  inter.triggerMouseButton(
    f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::LEFT);
  inter.triggerMouseButton(
    f3d::interactor::InputAction::PRESS, f3d::interactor::MouseButton::RIGHT);
  inter.triggerMouseButton(
    f3d::interactor::InputAction::RELEASE, f3d::interactor::MouseButton::RIGHT);
  return EXIT_SUCCESS;
}
