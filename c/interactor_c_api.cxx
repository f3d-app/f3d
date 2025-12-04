#include "interactor_c_api.h"
#include "interactor.h"
#include <string>

//----------------------------------------------------------------------------
void f3d_interactor_toggle_animation(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->toggleAnimation();
}

//----------------------------------------------------------------------------
void f3d_interactor_start_animation(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->startAnimation();
}

//----------------------------------------------------------------------------
void f3d_interactor_stop_animation(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->stopAnimation();
}

//----------------------------------------------------------------------------
int f3d_interactor_is_playing_animation(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return 0;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  return cpp_interactor->isPlayingAnimation() ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_interactor_enable_camera_movement(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->enableCameraMovement();
}

//----------------------------------------------------------------------------
void f3d_interactor_disable_camera_movement(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->disableCameraMovement();
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_mod_update(
  f3d_interactor_t* interactor, f3d_interactor_input_modifier_t mod)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::InputModifier cpp_mod = static_cast<f3d::interactor::InputModifier>(mod);
  cpp_interactor->triggerModUpdate(cpp_mod);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_mouse_button(f3d_interactor_t* interactor,
  f3d_interactor_input_action_t action, f3d_interactor_mouse_button_t button)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::InputAction cpp_action = static_cast<f3d::interactor::InputAction>(action);
  f3d::interactor::MouseButton cpp_button = static_cast<f3d::interactor::MouseButton>(button);
  cpp_interactor->triggerMouseButton(cpp_action, cpp_button);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_mouse_position(f3d_interactor_t* interactor, double xpos, double ypos)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->triggerMousePosition(xpos, ypos);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_mouse_wheel(
  f3d_interactor_t* interactor, f3d_interactor_wheel_direction_t direction)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::WheelDirection cpp_direction =
    static_cast<f3d::interactor::WheelDirection>(direction);
  cpp_interactor->triggerMouseWheel(cpp_direction);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_keyboard_key(
  f3d_interactor_t* interactor, f3d_interactor_input_action_t action, const char* key_sym)
{
  if (!interactor || !key_sym)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::InputAction cpp_action = static_cast<f3d::interactor::InputAction>(action);
  cpp_interactor->triggerKeyboardKey(cpp_action, key_sym);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_text_character(f3d_interactor_t* interactor, unsigned int codepoint)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->triggerTextCharacter(codepoint);
}

//----------------------------------------------------------------------------
void f3d_interactor_trigger_event_loop(f3d_interactor_t* interactor, double delta_time)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->triggerEventLoop(delta_time);
}

//----------------------------------------------------------------------------
int f3d_interactor_play_interaction(
  f3d_interactor_t* interactor, const char* file_path, double delta_time)
{
  if (!interactor || !file_path)
  {
    return 0;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  return cpp_interactor->playInteraction(file_path, delta_time) ? 1 : 0;
}

//----------------------------------------------------------------------------
int f3d_interactor_record_interaction(f3d_interactor_t* interactor, const char* file_path)
{
  if (!interactor || !file_path)
  {
    return 0;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  return cpp_interactor->recordInteraction(file_path) ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_interactor_start_with_callback(f3d_interactor_t* interactor, double delta_time,
  f3d_interactor_callback_t callback, void* user_data)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);

  if (callback)
  {
    cpp_interactor->start(delta_time, [callback, user_data]() { callback(user_data); });
  }
  else
  {
    cpp_interactor->start(delta_time, nullptr);
  }
}

//----------------------------------------------------------------------------
void f3d_interactor_start(f3d_interactor_t* interactor, double delta_time)
{
  f3d_interactor_start_with_callback(interactor, delta_time, nullptr, nullptr);
}

//----------------------------------------------------------------------------
void f3d_interactor_stop(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->stop();
}

//----------------------------------------------------------------------------
void f3d_interactor_request_render(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->requestRender();
}

//----------------------------------------------------------------------------
void f3d_interactor_request_stop(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->requestStop();
}

//----------------------------------------------------------------------------
void f3d_interactor_init_commands(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->initCommands();
}

//----------------------------------------------------------------------------
void f3d_interactor_remove_command(f3d_interactor_t* interactor, const char* action)
{
  if (!interactor || !action)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->removeCommand(action);
}

//----------------------------------------------------------------------------
int f3d_interactor_trigger_command(
  f3d_interactor_t* interactor, const char* command, int keep_comments)
{
  if (!interactor || !command)
  {
    return 0;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  return cpp_interactor->triggerCommand(command, keep_comments != 0) ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_interactor_init_bindings(f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  cpp_interactor->initBindings();
}

//----------------------------------------------------------------------------
void f3d_interactor_remove_binding(f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind)
{
  if (!interactor || !bind)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interaction_bind_t cpp_bind;
  cpp_bind.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(bind->mod);
  cpp_bind.inter = bind->inter;
  cpp_interactor->removeBinding(cpp_bind);
}

//----------------------------------------------------------------------------
f3d_interactor_binding_type_t f3d_interactor_get_binding_type(
  f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind)
{
  if (!interactor || !bind)
  {
    return F3D_INTERACTOR_BINDING_OTHER;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interaction_bind_t cpp_bind;
  cpp_bind.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(bind->mod);
  cpp_bind.inter = bind->inter;
  f3d::interactor::BindingType cpp_type = cpp_interactor->getBindingType(cpp_bind);
  return static_cast<f3d_interactor_binding_type_t>(cpp_type);
}
