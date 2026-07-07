#include "pseudo_unit_test.h"

#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <window_c_api.h>

#include <string.h>

static void stop_callback(void* user_data)
{
  f3d_interactor_t* interactor = (f3d_interactor_t*)user_data;
  f3d_interactor_request_stop(interactor);
}

int test_interactor()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_engine_t* engine = f3d_engine_create(1);
  f3d_test_check(&test, "engine created", engine != NULL);
  if (!engine)
  {
    return f3d_test_result(&test);
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  f3d_test_check(&test, "window retrieved", window != NULL);
  if (window)
  {
    f3d_window_render(window);
  }

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  f3d_test_check(&test, "interactor retrieved", interactor != NULL);
  if (!interactor)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  f3d_interactor_toggle_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);

  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_BACKWARD);
  int playing = f3d_interactor_is_playing_animation(interactor);
  f3d_test_check(&test, "animation is playing after start", playing != 0);

  f3d_interactor_animation_direction_t direction =
    f3d_interactor_get_animation_direction(interactor);
  f3d_test_check_int(&test, "animation direction matches what was started", direction,
    F3D_INTERACTOR_ANIMATION_BACKWARD);

  f3d_interactor_stop_animation(interactor);
  playing = f3d_interactor_is_playing_animation(interactor);
  f3d_test_check(&test, "animation is not playing after stop", playing == 0);

  f3d_interactor_enable_camera_movement(interactor);
  f3d_interactor_disable_camera_movement(interactor);

  // these are input-simulation calls with no observable return value, kept only
  // to make sure they don't crash
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

  f3d_interactor_trigger_notification(interactor, "foo", "bar", 3.0);

  int play_ret = f3d_interactor_play_interaction(interactor, "/nonexistent.log", 1.0 / 30.0);
  f3d_test_check(&test, "playing a nonexistent interaction file reports failure", play_ret == 0);

  int record_ret = f3d_interactor_record_interaction(interactor, "/tmp/test_interaction.log");
  f3d_test_check(&test, "recording interaction to /tmp reports success", record_ret == 1);

  f3d_interactor_request_render(interactor);
  f3d_interactor_request_stop(interactor);

  f3d_interactor_init_commands(interactor);
  f3d_interactor_remove_command(interactor, "test_action");

  // "print Test" is not a valid/registered command in this context (no such option
  // exists), so triggering it is expected to fail, not succeed
  int cmd_ret1 = f3d_interactor_trigger_command(interactor, "print Test", 0);
  f3d_test_check(&test, "triggering an invalid command reports failure", cmd_ret1 == 0);

  int cmd_ret2 = f3d_interactor_trigger_command(interactor, "print Test # comment", 1);
  f3d_test_check(
    &test, "triggering an invalid command with a comment reports failure", cmd_ret2 == 0);

  f3d_interactor_init_bindings(interactor);

  f3d_interactor_add_command(interactor, "test_action", NULL, NULL);
  int action_count = 0;
  char** actions = f3d_interactor_get_command_actions(interactor, &action_count);
  f3d_test_check(
    &test, "command actions include the one just added", actions != NULL && action_count >= 1);
  if (actions)
  {
    f3d_interactor_free_string_array(actions, action_count);
  }

  f3d_interaction_bind_t bind;
  bind.mod = F3D_INTERACTION_BIND_NONE;
  snprintf(bind.inter, sizeof(bind.inter), "t");

  char formatted[512];
  f3d_interaction_bind_format(&bind, formatted, sizeof(formatted));
  f3d_test_check(&test, "format() of a bind returns a non-empty string", strlen(formatted) > 0);

  f3d_interaction_bind_t ctrl_bind;
  ctrl_bind.mod = F3D_INTERACTION_BIND_CTRL;
  snprintf(ctrl_bind.inter, sizeof(ctrl_bind.inter), "A");

  char ctrl_formatted[512];
  f3d_interaction_bind_format(&ctrl_bind, ctrl_formatted, sizeof(ctrl_formatted));
  f3d_test_check(&test, "format() of a different bind gives a different string",
    strcmp(formatted, ctrl_formatted) != 0);

  f3d_interaction_bind_t parsed_bind;
  f3d_interaction_bind_parse("Shift+B", &parsed_bind);
  f3d_test_check_int(&test, "parse() of \"Shift+B\" extracts the Shift modifier",
    parsed_bind.mod, F3D_INTERACTION_BIND_SHIFT);
  f3d_test_check(
    &test, "parse() of \"Shift+B\" extracts the key part", strcmp(parsed_bind.inter, "B") == 0);

  int equals1 = f3d_interaction_bind_equals(&ctrl_bind, &parsed_bind);
  f3d_test_check(&test, "Ctrl+A and Shift+B binds are not equal", equals1 == 0);

  int less1 = f3d_interaction_bind_less_than(&ctrl_bind, &parsed_bind);
  (void)less1;

  const char* test_commands[] = { "test_action" };
  f3d_interactor_add_binding(
    interactor, &bind, test_commands, 1, "test_group", F3D_INTERACTOR_BINDING_CYCLIC, 1);

  int group_count = 0;
  char** groups = f3d_interactor_get_bind_groups(interactor, &group_count);
  f3d_test_check(
    &test, "bind groups include the one just added", groups != NULL && group_count >= 1);
  if (groups)
  {
    f3d_interactor_free_string_array(groups, group_count);
  }

  int bind_count = 0;
  f3d_interaction_bind_t* binds_for_group =
    f3d_interactor_get_binds_for_group(interactor, "test_group", &bind_count);
  f3d_test_check(&test, "test_group has exactly the one bind added", bind_count == 1);
  if (binds_for_group)
  {
    f3d_interactor_free_bind_array(binds_for_group);
  }

  int all_bind_count = 0;
  f3d_interaction_bind_t* all_binds = f3d_interactor_get_binds(interactor, &all_bind_count);
  f3d_test_check(&test, "get_binds() reports at least the one bind added", all_bind_count >= 1);
  if (all_binds)
  {
    f3d_interactor_free_bind_array(all_binds);
  }

  f3d_binding_documentation_t doc;
  f3d_interactor_get_binding_documentation(interactor, &bind, &doc);


  f3d_interactor_binding_type_t binding_type = f3d_interactor_get_binding_type(interactor, &bind);
  f3d_test_check_int(
    &test, "binding type matches what was added", binding_type, F3D_INTERACTOR_BINDING_CYCLIC);

  f3d_interactor_remove_binding(interactor, &bind);

  bind_count = 0;
  binds_for_group = f3d_interactor_get_binds_for_group(interactor, "test_group", &bind_count);
  f3d_test_check(&test, "test_group has no binds after removal", bind_count == 0);
  if (binds_for_group)
  {
    f3d_interactor_free_bind_array(binds_for_group);
  }

  f3d_interactor_binding_type_t removed_type =
    f3d_interactor_get_binding_type(interactor, &bind);
  f3d_test_check_int(
    &test, "binding type is OTHER after removal", removed_type, F3D_INTERACTOR_BINDING_OTHER);

  f3d_interactor_set_event_loop_user_callback(interactor, stop_callback, interactor);
  f3d_interactor_start(interactor, 0.01);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
