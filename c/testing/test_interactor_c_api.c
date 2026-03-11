#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <window_c_api.h>

#include <stdio.h>
#include <string.h>

static void stop_callback(void* user_data)
{
  f3d_interactor_t* interactor = (f3d_interactor_t*)user_data;
  f3d_interactor_request_stop(interactor);
}

int test_interactor_c_api()
{
  f3d_engine_t* engine = f3d_engine_create(1);
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (window)
  {
    f3d_window_render(window);
  }

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  if (!interactor)
  {
    puts("[ERROR] Failed to get interactor");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_interactor_toggle_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_BACKWARD);
  int playing = f3d_interactor_is_playing_animation(interactor);
  (void)playing;
  f3d_interactor_animation_direction_t direction =
    f3d_interactor_get_animation_direction(interactor);
  (void)direction;
  f3d_interactor_stop_animation(interactor);

  f3d_interactor_enable_camera_movement(interactor);
  f3d_interactor_disable_camera_movement(interactor);

  f3d_interactor_trigger_mod_update(interactor, F3D_INTERACTOR_INPUT_CTRL);
  f3d_interactor_trigger_mouse_button(
    interactor, F3D_INTERACTOR_INPUT_PRESS, F3D_INTERACTOR_MOUSE_LEFT);
  f3d_interactor_trigger_mouse_button(
    interactor, F3D_INTERACTOR_INPUT_RELEASE, F3D_INTERACTOR_MOUSE_RIGHT);
  f3d_interactor_trigger_mouse_position(interactor, 100.0, 200.0);
  f3d_interactor_trigger_mouse_wheel(interactor, F3D_INTERACTOR_WHEEL_FORWARD);
  f3d_interactor_trigger_mouse_wheel(interactor, F3D_INTERACTOR_WHEEL_BACKWARD);
  f3d_interactor_trigger_keyboard_key(interactor, F3D_INTERACTOR_INPUT_PRESS, "a");
  f3d_interactor_trigger_keyboard_key(interactor, F3D_INTERACTOR_INPUT_RELEASE, "b");
  f3d_interactor_trigger_text_character(interactor, 65);

  f3d_interactor_trigger_event_loop(interactor, 0.016);

  f3d_interactor_play_interaction(interactor, "/nonexistent.log", 1.0 / 30.0);
  f3d_interactor_record_interaction(interactor, "/tmp/test_interaction.log");

  f3d_interactor_request_render(interactor);
  f3d_interactor_request_stop(interactor);

  f3d_interactor_init_commands(interactor);
  f3d_interactor_remove_command(interactor, "test_action");
  f3d_interactor_trigger_command(interactor, "print Test", 0);
  f3d_interactor_trigger_command(interactor, "print Test # comment", 1);

  f3d_interactor_init_bindings(interactor);

  f3d_interactor_add_command(interactor, "test_action", NULL, NULL);
  int action_count = 0;
  char** actions = f3d_interactor_get_command_actions(interactor, &action_count);
  if (actions)
  {
    f3d_interactor_free_string_array(actions, action_count);
  }

  f3d_interaction_bind_t bind;
  bind.mod = F3D_INTERACTION_BIND_NONE;
  snprintf(bind.inter, sizeof(bind.inter), "t");

  char formatted[512];
  f3d_interaction_bind_format(&bind, formatted, sizeof(formatted));

  f3d_interaction_bind_t ctrl_bind;
  ctrl_bind.mod = F3D_INTERACTION_BIND_CTRL;
  snprintf(ctrl_bind.inter, sizeof(ctrl_bind.inter), "A");
  f3d_interaction_bind_format(&ctrl_bind, formatted, sizeof(formatted));

  f3d_interaction_bind_t parsed_bind;
  f3d_interaction_bind_parse("Shift+B", &parsed_bind);

  int equals1 = f3d_interaction_bind_equals(&ctrl_bind, &parsed_bind);
  (void)equals1;

  int less1 = f3d_interaction_bind_less_than(&ctrl_bind, &parsed_bind);
  (void)less1;

  const char* test_commands[] = { "test_action" };
  f3d_interactor_add_binding(interactor, &bind, test_commands, 1, "test_group");

  int group_count = 0;
  char** groups = f3d_interactor_get_bind_groups(interactor, &group_count);
  if (groups)
  {
    f3d_interactor_free_string_array(groups, group_count);
  }

  int bind_count = 0;
  f3d_interaction_bind_t* binds_for_group =
    f3d_interactor_get_binds_for_group(interactor, "test_group", &bind_count);
  if (binds_for_group)
  {
    f3d_interactor_free_bind_array(binds_for_group);
  }

  int all_bind_count = 0;
  f3d_interaction_bind_t* all_binds = f3d_interactor_get_binds(interactor, &all_bind_count);
  if (all_binds)
  {
    f3d_interactor_free_bind_array(all_binds);
  }

  f3d_binding_documentation_t doc;
  f3d_interactor_get_binding_documentation(interactor, &bind, &doc);

  f3d_interactor_binding_type_t binding_type = f3d_interactor_get_binding_type(interactor, &bind);
  (void)binding_type;

  f3d_interactor_remove_binding(interactor, &bind);

  f3d_interactor_start_with_callback(interactor, 0.01, stop_callback, interactor);

  f3d_engine_delete(engine);
  return 0;
}
