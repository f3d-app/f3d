#include "interactor_c_api.h"
#include "interactor.h"
#include <algorithm>
#include <cstring>
#include <string>

//----------------------------------------------------------------------------
void f3d_interaction_bind_format(const f3d_interaction_bind_t* bind, char* output, int output_size)
{
  if (!bind || !output || output_size <= 0)
  {
    return;
  }

  f3d::interaction_bind_t cpp_bind;
  cpp_bind.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(bind->mod);
  cpp_bind.inter = bind->inter;

  std::string formatted = cpp_bind.format();
  std::strncpy(output, formatted.c_str(), output_size - 1);
  output[output_size - 1] = '\0';
}

//----------------------------------------------------------------------------
void f3d_interaction_bind_parse(const char* str, f3d_interaction_bind_t* bind)
{
  if (!str || !bind)
  {
    return;
  }

  f3d::interaction_bind_t cpp_bind = f3d::interaction_bind_t::parse(str);

  bind->mod = static_cast<f3d_interaction_bind_modifier_keys_t>(cpp_bind.mod);
  std::strncpy(bind->inter, cpp_bind.inter.c_str(), sizeof(bind->inter) - 1);
  bind->inter[sizeof(bind->inter) - 1] = '\0';
}

//----------------------------------------------------------------------------
int f3d_interaction_bind_less_than(
  const f3d_interaction_bind_t* lhs, const f3d_interaction_bind_t* rhs)
{
  if (!lhs || !rhs)
  {
    return 0;
  }

  f3d::interaction_bind_t cpp_lhs;
  cpp_lhs.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(lhs->mod);
  cpp_lhs.inter = lhs->inter;

  f3d::interaction_bind_t cpp_rhs;
  cpp_rhs.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(rhs->mod);
  cpp_rhs.inter = rhs->inter;

  return cpp_lhs < cpp_rhs ? 1 : 0;
}

//----------------------------------------------------------------------------
int f3d_interaction_bind_equals(
  const f3d_interaction_bind_t* lhs, const f3d_interaction_bind_t* rhs)
{
  if (!lhs || !rhs)
  {
    return 0;
  }

  f3d::interaction_bind_t cpp_lhs;
  cpp_lhs.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(lhs->mod);
  cpp_lhs.inter = lhs->inter;

  f3d::interaction_bind_t cpp_rhs;
  cpp_rhs.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(rhs->mod);
  cpp_rhs.inter = rhs->inter;

  return cpp_lhs == cpp_rhs ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_interactor_toggle_animation(
  f3d_interactor_t* interactor, f3d_interactor_animation_direction_t direction)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::AnimationDirection cpp_direction =
    static_cast<f3d::interactor::AnimationDirection>(direction);
  cpp_interactor->toggleAnimation(cpp_direction);
}

//----------------------------------------------------------------------------
void f3d_interactor_start_animation(
  f3d_interactor_t* interactor, f3d_interactor_animation_direction_t direction)
{
  if (!interactor)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::AnimationDirection cpp_direction =
    static_cast<f3d::interactor::AnimationDirection>(direction);
  cpp_interactor->startAnimation(cpp_direction);
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
f3d_interactor_animation_direction_t f3d_interactor_get_animation_direction(
  f3d_interactor_t* interactor)
{
  if (!interactor)
  {
    return F3D_INTERACTOR_ANIMATION_FORWARD;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interactor::AnimationDirection cpp_direction = cpp_interactor->getAnimationDirection();
  return static_cast<f3d_interactor_animation_direction_t>(cpp_direction);
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
void f3d_interactor_add_command(f3d_interactor_t* interactor, const char* action,
  f3d_interactor_command_callback_t callback, void* user_data)
{
  if (!interactor || !action || !callback)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);

  auto cpp_callback = [callback, user_data](const std::vector<std::string>& args)
  {
    std::vector<const char*> c_args;
    c_args.resize(args.size());
    std::transform(
      args.begin(), args.end(), c_args.begin(), [](const std::string& s) { return s.c_str(); });
    callback(c_args.data(), static_cast<int>(c_args.size()), user_data);
  };

  cpp_interactor->addCommand(action, cpp_callback);
}

//----------------------------------------------------------------------------
char** f3d_interactor_get_command_actions(f3d_interactor_t* interactor, int* count)
{
  if (!interactor || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  std::vector<std::string> actions = cpp_interactor->getCommandActions();

  *count = static_cast<int>(actions.size());
  if (actions.empty())
  {
    return nullptr;
  }

  char** result = new char*[actions.size()];

  for (size_t i = 0; i < actions.size(); ++i)
  {
    result[i] = new char[actions[i].length() + 1];
    std::strcpy(result[i], actions[i].c_str());
  }

  return result;
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
void f3d_interactor_add_binding(f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind,
  const char** commands, int command_count, const char* group)
{
  if (!interactor || !bind || !commands || command_count <= 0)
  {
    return;
  }

  f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);

  f3d::interaction_bind_t cpp_bind;
  cpp_bind.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(bind->mod);
  cpp_bind.inter = bind->inter;

  std::vector<std::string> cpp_commands;
  cpp_commands.reserve(command_count);
  for (int i = 0; i < command_count; ++i)
  {
    cpp_commands.push_back(commands[i]);
  }

  std::string cpp_group = group ? group : "";

  cpp_interactor->addBinding(cpp_bind, cpp_commands, cpp_group);
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
char** f3d_interactor_get_bind_groups(f3d_interactor_t* interactor, int* count)
{
  if (!interactor || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  std::vector<std::string> groups = cpp_interactor->getBindGroups();

  *count = static_cast<int>(groups.size());
  if (groups.empty())
  {
    return nullptr;
  }

  char** result = new char*[groups.size()];

  for (size_t i = 0; i < groups.size(); ++i)
  {
    result[i] = new char[groups[i].length() + 1];
    std::strcpy(result[i], groups[i].c_str());
  }

  return result;
}

//----------------------------------------------------------------------------
f3d_interaction_bind_t* f3d_interactor_get_binds_for_group(
  f3d_interactor_t* interactor, const char* group, int* count)
{
  if (!interactor || !group || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  std::vector<f3d::interaction_bind_t> binds = cpp_interactor->getBindsForGroup(group);

  *count = static_cast<int>(binds.size());
  if (binds.empty())
  {
    return nullptr;
  }

  f3d_interaction_bind_t* result = new f3d_interaction_bind_t[binds.size()];

  for (size_t i = 0; i < binds.size(); ++i)
  {
    result[i].mod = static_cast<f3d_interaction_bind_modifier_keys_t>(binds[i].mod);
    std::strncpy(result[i].inter, binds[i].inter.c_str(), sizeof(result[i].inter) - 1);
    result[i].inter[sizeof(result[i].inter) - 1] = '\0';
  }

  return result;
}

//----------------------------------------------------------------------------
f3d_interaction_bind_t* f3d_interactor_get_binds(f3d_interactor_t* interactor, int* count)
{
  if (!interactor || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  std::vector<f3d::interaction_bind_t> binds = cpp_interactor->getBinds();

  *count = static_cast<int>(binds.size());
  if (binds.empty())
  {
    return nullptr;
  }

  f3d_interaction_bind_t* result = new f3d_interaction_bind_t[binds.size()];

  for (size_t i = 0; i < binds.size(); ++i)
  {
    result[i].mod = static_cast<f3d_interaction_bind_modifier_keys_t>(binds[i].mod);
    std::strncpy(result[i].inter, binds[i].inter.c_str(), sizeof(result[i].inter) - 1);
    result[i].inter[sizeof(result[i].inter) - 1] = '\0';
  }

  return result;
}

//----------------------------------------------------------------------------
void f3d_interactor_get_binding_documentation(f3d_interactor_t* interactor,
  const f3d_interaction_bind_t* bind, f3d_binding_documentation_t* doc)
{
  if (!interactor || !bind || !doc)
  {
    return;
  }

  const f3d::interactor* cpp_interactor = reinterpret_cast<f3d::interactor*>(interactor);
  f3d::interaction_bind_t cpp_bind;
  cpp_bind.mod = static_cast<f3d::interaction_bind_t::ModifierKeys>(bind->mod);
  cpp_bind.inter = bind->inter;

  auto [doc_str, value_str] = cpp_interactor->getBindingDocumentation(cpp_bind);

  std::strncpy(doc->doc, doc_str.c_str(), sizeof(doc->doc) - 1);
  doc->doc[sizeof(doc->doc) - 1] = '\0';

  std::strncpy(doc->value, value_str.c_str(), sizeof(doc->value) - 1);
  doc->value[sizeof(doc->value) - 1] = '\0';
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

//----------------------------------------------------------------------------
void f3d_interactor_free_string_array(char** array, int count)
{
  if (!array)
  {
    return;
  }

  for (int i = 0; i < count; ++i)
  {
    delete[] array[i];
  }
  delete[] array;
}

//----------------------------------------------------------------------------
void f3d_interactor_free_bind_array(f3d_interaction_bind_t* array)
{
  delete[] array;
}
