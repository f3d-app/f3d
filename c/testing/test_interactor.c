#include "pseudo_unit_test.h"
#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <window_c_api.h>
#include <string.h>

static int callback_called = 0;

static void test_event_loop_callback(void* user_data)
{
  callback_called++;

  if (user_data)
  {
    (*(int*)user_data)++;
  }
}

static int command_callback_called = 0;

static void test_command_callback(
  const char** args,
  int argc,
  void* user_data)
{
  (void)args;
  (void)argc;

  command_callback_called++;

  if (user_data)
  {
    (*(int*)user_data)++;
  }
}

int test_interactor()
{
  f3d_test_t test;
  f3d_test_init(&test);

  // f3d_interaction_bind_format tests
  char output[256];

  f3d_interaction_bind_format(NULL, output, sizeof(output));
  f3d_test_check(&test, "interaction_bind_format(NULL bind)", 1);

  f3d_interaction_bind_t bind = { 0 };
  bind.mod = F3D_INTERACTION_BIND_NONE;
  strcpy(bind.inter, "A");

  f3d_interaction_bind_format(&bind, NULL, sizeof(output));
  f3d_test_check(&test, "interaction_bind_format(NULL output)", 1);

  f3d_interaction_bind_format(&bind, output, 0);
  f3d_test_check(&test, "interaction_bind_format(zero output size)", 1);

  memset(output, 0, sizeof(output));
  f3d_interaction_bind_format(&bind, output, sizeof(output));

  f3d_test_check(&test, "interaction_bind_format(valid)", strlen(output) > 0);

  char small_output[2];
  memset(small_output, 0, sizeof(small_output));

  f3d_interaction_bind_format(&bind, small_output, sizeof(small_output));

  f3d_test_check(&test, "interaction_bind_format(truncated)", small_output[1] == '\0');

  // f3d_interaction_bind_parse tests
  f3d_interaction_bind_t parsed = { 0 };

  f3d_interaction_bind_parse(NULL, &parsed);
  f3d_test_check(&test, "interaction_bind_parse(NULL string)", 1);

  f3d_interaction_bind_parse(output, NULL);
  f3d_test_check(&test, "interaction_bind_parse(NULL bind)", 1);

  memset(&parsed, 0, sizeof(parsed));
  f3d_interaction_bind_parse(output, &parsed);

  f3d_test_check_int(&test, "interaction_bind_parse(modifier)", parsed.mod, bind.mod);

  f3d_test_check_string(&test, "interaction_bind_parse(interaction)", parsed.inter, bind.inter);

  // f3d_interaction_bind_less_than tests
  f3d_test_check_int(&test, "interaction_bind_less_than(NULL lhs)", f3d_interaction_bind_less_than(NULL, &bind), 0);
  f3d_test_check_int(&test, "interaction_bind_less_than(NULL rhs)", f3d_interaction_bind_less_than(&bind, NULL), 0);

  f3d_interaction_bind_t bind_a = { 0 };
  bind_a.mod = F3D_INTERACTION_BIND_NONE;
  strcpy(bind_a.inter, "A");

  f3d_interaction_bind_t bind_b = { 0 };
  bind_b.mod = F3D_INTERACTION_BIND_NONE;
  strcpy(bind_b.inter, "B");

  f3d_test_check_int(&test, "interaction_bind_less_than(interaction)", 
    f3d_interaction_bind_less_than(&bind_a, &bind_b), 1);

  f3d_test_check_int(&test, "interaction_bind_less_than(reverse interaction)",
    f3d_interaction_bind_less_than(&bind_b, &bind_a), 0);

  f3d_interaction_bind_t bind_ctrl = { 0 };
  bind_ctrl.mod = F3D_INTERACTION_BIND_CTRL;
  strcpy(bind_ctrl.inter, "A");

  f3d_test_check_int(&test, "interaction_bind_less_than(modifier)",
    f3d_interaction_bind_less_than(&bind_a, &bind_ctrl), 1);

  f3d_test_check_int(&test, "interaction_bind_less_than(equal)",
    f3d_interaction_bind_less_than(&bind_a, &bind_a), 0);

  // f3d_interaction_bind_equals tests
  f3d_test_check_int(&test, "interaction_bind_equals(NULL lhs)",
    f3d_interaction_bind_equals(NULL, &bind), 0);

  f3d_test_check_int(&test, "interaction_bind_equals(NULL rhs)",
    f3d_interaction_bind_equals(&bind, NULL), 0);

  f3d_test_check_int(&test, "interaction_bind_equals(equal)",
    f3d_interaction_bind_equals(&bind_a, &bind_a), 1);

  f3d_test_check_int(&test, "interaction_bind_equals(different interaction)",
    f3d_interaction_bind_equals(&bind_a, &bind_b), 0);

  f3d_test_check_int(&test, "interaction_bind_equals(different modifier)",
    f3d_interaction_bind_equals(&bind_a, &bind_ctrl), 0);

  //engine creation for further tests
  f3d_engine_t* engine = f3d_engine_create(1);
  f3d_test_check_ptr(&test, "engine created", engine);
  if (!engine)
  {
    return f3d_test_result(&test);
  }

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  f3d_test_check_ptr(&test, "interactor retrieved", interactor);
  if (!interactor)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  // f3d_interactor_toggle_animation tests
  f3d_interactor_toggle_animation(NULL, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_test_check(&test, "interactor_toggle_animation(NULL)", 1);

  f3d_interactor_toggle_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_test_check(&test, "interactor_toggle_animation(forward)", 1);

  // f3d_interactor_start_animation tests
  f3d_interactor_start_animation(NULL, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_test_check(&test, "interactor_start_animation(NULL)", 1);

  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_test_check(&test, "interactor_start_animation(valid)", 1);

  // f3d_interactor_stop_animation tests
  f3d_interactor_stop_animation(NULL);
  f3d_test_check(&test, "interactor_stop_animation(NULL)", 1);

  f3d_interactor_stop_animation(interactor);
  f3d_test_check(&test, "interactor_stop_animation(valid)", 1);

  // f3d_interactor_is_playing_animation tests
  f3d_test_check_int(&test, "interactor_is_playing_animation(NULL)", f3d_interactor_is_playing_animation(NULL), 0);
  
  f3d_interactor_stop_animation(interactor);
  f3d_test_check_int(&test, "interactor_is_playing_animation(stopped)",
    f3d_interactor_is_playing_animation(interactor), 0);

  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);
  f3d_test_check_int(&test, "interactor_is_playing_animation(started)",
    f3d_interactor_is_playing_animation(interactor), 1);

  f3d_interactor_stop_animation(interactor);

  // f3d_interactor_get_animation_direction tests
  f3d_test_check_int(&test, "interactor_get_animation_direction(NULL)",
    f3d_interactor_get_animation_direction(NULL), F3D_INTERACTOR_ANIMATION_FORWARD);

  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_FORWARD);

  f3d_test_check_int(&test, "interactor_get_animation_direction(forward)",
    f3d_interactor_get_animation_direction(interactor), F3D_INTERACTOR_ANIMATION_FORWARD);

  f3d_interactor_start_animation(interactor, F3D_INTERACTOR_ANIMATION_BACKWARD);

  f3d_test_check_int(&test, "interactor_get_animation_direction(backward)",
    f3d_interactor_get_animation_direction(interactor), F3D_INTERACTOR_ANIMATION_BACKWARD);

  f3d_interactor_stop_animation(interactor);

  // f3d_interactor_enable_camera_movement tests
  f3d_interactor_enable_camera_movement(NULL);
  f3d_test_check(&test, "interactor_enable_camera_movement(NULL)", 1);

  f3d_interactor_enable_camera_movement(interactor);
  f3d_test_check(&test, "interactor_enable_camera_movement(valid)", 1);

  // f3d_interactor_disable_camera_movement tests
  f3d_interactor_disable_camera_movement(NULL);
  f3d_test_check(&test, "interactor_disable_camera_movement(NULL)", 1);

  f3d_interactor_disable_camera_movement(interactor);
  f3d_test_check(&test, "interactor_disable_camera_movement(valid)", 1);

  // f3d_interactor_trigger_mod_update tests
  f3d_interactor_trigger_mod_update(NULL, F3D_INTERACTOR_INPUT_CTRL);
  f3d_test_check(&test, "interactor_trigger_mod_update(NULL)", 1);

  f3d_interactor_trigger_mod_update(interactor, F3D_INTERACTOR_INPUT_CTRL);
  f3d_test_check(&test, "interactor_trigger_mod_update(valid)", 1);

  // f3d_interactor_trigger_mouse_button tests
  f3d_interactor_trigger_mouse_button(NULL, F3D_INTERACTOR_INPUT_PRESS, F3D_INTERACTOR_MOUSE_LEFT);
  f3d_test_check(&test, "interactor_trigger_mouse_button(NULL)", 1);

  f3d_interactor_trigger_mouse_button(interactor, F3D_INTERACTOR_INPUT_PRESS, F3D_INTERACTOR_MOUSE_LEFT);
  f3d_test_check(&test, "interactor_trigger_mouse_button(valid)", 1);

  // f3d_interactor_trigger_mouse_position tests
  f3d_interactor_trigger_mouse_position(NULL, 100.0, 200.0);
  f3d_test_check(&test, "interactor_trigger_mouse_position(NULL)", 1);

  f3d_interactor_trigger_mouse_position(interactor, 100.0, 200.0);
  f3d_test_check(&test, "interactor_trigger_mouse_position(valid)", 1);

  // f3d_interactor_trigger_mouse_wheel tests
  f3d_interactor_trigger_mouse_wheel(NULL, F3D_INTERACTOR_WHEEL_FORWARD);
  f3d_test_check(&test, "interactor_trigger_mouse_wheel(NULL)", 1);

  f3d_interactor_trigger_mouse_wheel(interactor, F3D_INTERACTOR_WHEEL_FORWARD);
  f3d_test_check(&test, "interactor_trigger_mouse_wheel(valid)", 1);

  // f3d_interactor_trigger_keyboard_key tests
  f3d_interactor_trigger_keyboard_key(NULL, F3D_INTERACTOR_INPUT_PRESS, "A");
  f3d_test_check(&test, "interactor_trigger_keyboard_key(NULL interactor)", 1);

  f3d_interactor_trigger_keyboard_key(interactor, F3D_INTERACTOR_INPUT_PRESS, NULL);
  f3d_test_check(&test, "interactor_trigger_keyboard_key(NULL key)", 1);

  f3d_interactor_trigger_keyboard_key(interactor, F3D_INTERACTOR_INPUT_PRESS, "A");
  f3d_test_check(&test, "interactor_trigger_keyboard_key(valid)", 1);

  // f3d_interactor_trigger_text_character tests
  f3d_interactor_trigger_text_character(NULL, 'A');
  f3d_test_check(&test, "interactor_trigger_text_character(NULL)", 1);

  f3d_interactor_trigger_text_character(interactor, 'A');
  f3d_test_check(&test, "interactor_trigger_text_character(valid)", 1);

  // f3d_interactor_trigger_event_loop tests
  f3d_interactor_trigger_event_loop(NULL, 0.016);
  f3d_test_check(&test, "interactor_trigger_event_loop(NULL)", 1);

  f3d_interactor_trigger_event_loop(interactor, 0.016);
  f3d_test_check(&test, "interactor_trigger_event_loop(valid)", 1);

  // f3d_interactor_play_interaction tests
  f3d_test_check_int(&test, "interactor_play_interaction(NULL interactor)",
    f3d_interactor_play_interaction(NULL, F3D_TESTING_BUILD_DIR "TestSDKInteractorCallBack.log", 1.0 / 30.0), 0);

  f3d_test_check_int(&test, "interactor_play_interaction(NULL path)",
    f3d_interactor_play_interaction(interactor, NULL, 1.0 / 30.0),0);

  f3d_test_check_int(&test, "interactor_play_interaction(invalid path)",
    f3d_interactor_play_interaction(interactor, "/invalid/path/interaction.log", 1.0 / 30.0), 0);

  f3d_test_check_int(&test, "interactor_play_interaction(valid)",
    f3d_interactor_play_interaction(interactor, F3D_TESTING_BUILD_DIR "TestSDKInteractorCallBack.log", 1.0 / 30.0), 1);

  // f3d_interactor_record_interaction tests
  f3d_test_check_int(&test, "interactor_record_interaction(NULL interactor)",
    f3d_interactor_record_interaction(NULL, F3D_TESTING_TEMP_DIR "interaction.log"), 0);

  f3d_test_check_int(&test, "interactor_record_interaction(NULL path)",
    f3d_interactor_record_interaction(interactor, NULL), 0);

  f3d_test_check_int(&test, "interactor_record_interaction(valid)",
    f3d_interactor_record_interaction(interactor, F3D_TESTING_TEMP_DIR "interaction.log"), 1);

  f3d_test_check_int(&test, "interactor_record_interaction(invalid path)",
    f3d_interactor_record_interaction(interactor, "/invalid/path/interaction.log"), 0);
 
  // f3d_interactor_set_event_loop_user_callback tests
  int callback_data = 0;

  f3d_interactor_set_event_loop_user_callback(NULL, test_event_loop_callback, &callback_data);
  f3d_test_check(&test, "interactor_set_event_loop_user_callback(NULL)", 1);

  f3d_interactor_set_event_loop_user_callback(interactor, test_event_loop_callback, &callback_data);
  f3d_test_check(&test, "interactor_set_event_loop_user_callback(valid)", 1);

  // f3d_interactor_start tests
  f3d_interactor_start(NULL, 1.0 / 30.0);
  f3d_test_check(&test, "interactor_start(NULL)", 1);

  // f3d_interactor_stop tests
  f3d_interactor_stop(NULL);
  f3d_test_check(&test, "interactor_stop(NULL)", 1);

  f3d_interactor_stop(interactor);
  f3d_test_check(&test, "interactor_stop(valid)", 1);

  // f3d_interactor_request_render tests
  f3d_interactor_request_render(NULL);
  f3d_test_check(&test, "interactor_request_render(NULL)", 1);

  f3d_interactor_request_render(interactor);
  f3d_test_check(&test, "interactor_request_render(valid)", 1);

  // f3d_interactor_request_stop tests
  f3d_interactor_request_stop(NULL);
  f3d_test_check(&test, "interactor_request_stop(NULL)", 1);

  f3d_interactor_request_stop(interactor);
  f3d_test_check(&test, "interactor_request_stop(valid)", 1);

  // f3d_interactor_init_commands tests
  f3d_interactor_init_commands(NULL);
  f3d_test_check(&test, "interactor_init_commands(NULL)", 1);

  f3d_interactor_init_commands(interactor);
  f3d_test_check(&test, "interactor_init_commands(valid)", 1);

  f3d_interactor_init_commands(interactor);
  f3d_test_check(&test, "interactor_init_commands(second call)", 1);

  // f3d_interactor_add_command tests
  int command_data = 0;

  f3d_interactor_add_command(NULL, "test_command", test_command_callback, &command_data);
  f3d_test_check(&test, "interactor_add_command(NULL interactor)", 1);

  f3d_interactor_add_command(interactor, NULL, test_command_callback, &command_data);
  f3d_test_check(&test, "interactor_add_command(NULL action)", 1);

  f3d_interactor_add_command(interactor, "test_command", NULL, &command_data);
  f3d_test_check(&test, "interactor_add_command(NULL callback)", 1);

  f3d_interactor_add_command(interactor, "test_command", test_command_callback, &command_data);
  f3d_test_check(&test, "interactor_add_command(valid)", 1);

  f3d_interactor_add_command(interactor, "test_command", test_command_callback, &command_data);
  f3d_test_check(&test, "interactor_add_command(duplicate)", 1);

  // f3d_interactor_get_command_actions tests
  int count = -1;

  f3d_test_check_null(&test, "interactor_get_command_actions(NULL interactor)",
    f3d_interactor_get_command_actions(NULL, &count));

  f3d_test_check_int(&test, "interactor_get_command_actions(NULL interactor count)", count, 0);

  count = -1;

  f3d_test_check_null(&test, "interactor_get_command_actions(NULL count)",
    f3d_interactor_get_command_actions(interactor, NULL));

  char** actions = f3d_interactor_get_command_actions(interactor, &count);

  f3d_test_check_ptr(&test, "interactor_get_command_actions(valid)", actions);

  f3d_test_check(&test, "interactor_get_command_actions(non-empty)", count > 0);

  if (actions)
  {
    f3d_interactor_free_string_array(actions, count);
  }

  // f3d_interactor_trigger_command tests
  f3d_test_check_int(&test, "interactor_trigger_command(NULL interactor)",
    f3d_interactor_trigger_command(NULL, "test_command", 0), 0);

  f3d_test_check_int(&test, "interactor_trigger_command(NULL command)",
    f3d_interactor_trigger_command(interactor, NULL, 0), 0);

  command_callback_called = 0;
  command_data = 0;

  f3d_test_check_int(&test, "interactor_trigger_command(valid)",
    f3d_interactor_trigger_command(interactor, "test_command", 0), 1);

  f3d_test_check_int(&test, "interactor_trigger_command(callback)",
    command_callback_called, 1);

  f3d_test_check_int(&test, "interactor_trigger_command(user_data)",
    command_data, 1);

  f3d_test_check_int(&test, "interactor_trigger_command(invalid)",
    f3d_interactor_trigger_command(interactor, "invalid_command", 0), 0);

  // f3d_interactor_remove_command tests
  f3d_interactor_remove_command(NULL, "test_command");
  f3d_test_check(&test, "interactor_remove_command(NULL interactor)", 1);

  f3d_interactor_remove_command(interactor, NULL);
  f3d_test_check(&test, "interactor_remove_command(NULL action)", 1);

  f3d_interactor_remove_command(interactor, "test_command");
  f3d_test_check(&test, "interactor_remove_command(valid)", 1);

  f3d_test_check_int(&test, "interactor_trigger_command(after remove)",
    f3d_interactor_trigger_command(interactor, "test_command", 0), 0);

  // f3d_interactor_init_bindings tests
  f3d_interactor_init_bindings(NULL);
  f3d_test_check(&test, "interactor_init_bindings(NULL)", 1);

  f3d_interactor_init_bindings(interactor);
  f3d_test_check(&test, "interactor_init_bindings(valid)", 1);

  f3d_interactor_init_bindings(interactor);
  f3d_test_check(&test, "interactor_init_bindings(second call)", 1);

  // f3d_interactor_add_binding tests
  f3d_interaction_bind_t custom_bind = { 0 };
  custom_bind.mod = F3D_INTERACTION_BIND_CTRL_SHIFT;
  strcpy(custom_bind.inter, "T");

  const char* commands[] = { "toggle ui.axis" };

  f3d_interactor_add_binding(NULL, &custom_bind, commands, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(NULL interactor)", 1);

  f3d_interactor_add_binding(interactor, NULL, commands, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(NULL bind)", 1);

  f3d_interactor_add_binding(interactor, &custom_bind, NULL, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(NULL commands)", 1);

  f3d_interactor_add_binding(interactor, &custom_bind, commands, 0, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(zero command count)", 1);

  f3d_interactor_add_binding(interactor, &custom_bind, commands, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(valid)", 1);

  f3d_interactor_add_binding(interactor, &custom_bind, commands, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);
  f3d_test_check(&test, "interactor_add_binding(duplicate)", 1);

  // f3d_interactor_remove_binding tests
  f3d_interactor_remove_binding(NULL, &custom_bind);
  f3d_test_check(&test, "interactor_remove_binding(NULL interactor)", 1);

  f3d_interactor_remove_binding(interactor, NULL);
  f3d_test_check(&test, "interactor_remove_binding(NULL bind)", 1);

  f3d_interactor_remove_binding(interactor, &custom_bind);
  f3d_test_check(&test, "interactor_remove_binding(valid)", 1);

  // Removing again should simply do nothing. 
  f3d_interactor_remove_binding(interactor, &custom_bind);
  f3d_test_check(&test, "interactor_remove_binding(missing)", 1);

  // Re-add for the remaining tests. 
  f3d_interactor_add_binding(interactor, &custom_bind, commands, 1, "Test", F3D_INTERACTOR_BINDING_TOGGLE, 1);

  // f3d_interactor_get_bind_groups tests
  int group_count = -1;

  f3d_test_check_null(&test, "interactor_get_bind_groups(NULL interactor)", 
    f3d_interactor_get_bind_groups(NULL, &group_count));

  f3d_test_check_int(&test, "interactor_get_bind_groups(NULL interactor count)", group_count, 0);

  group_count = -1;

  f3d_test_check_null(&test, "interactor_get_bind_groups(NULL count)",
    f3d_interactor_get_bind_groups(interactor, NULL));

  char** groups = f3d_interactor_get_bind_groups(interactor, &group_count);

  f3d_test_check_ptr(&test, "interactor_get_bind_groups(valid)", groups);

  f3d_test_check(&test, "interactor_get_bind_groups(non-empty)", group_count > 0);

  if (groups)
  {
    f3d_interactor_free_string_array(groups, group_count);
  }

  // f3d_interactor_get_binds_for_group tests
  int bind_count = -1;

  f3d_test_check_null(&test, "interactor_get_binds_for_group(NULL interactor)", 
    f3d_interactor_get_binds_for_group(NULL, "Test", &bind_count));

  f3d_test_check_int(&test, "interactor_get_binds_for_group(NULL interactor count)", bind_count, 0);

  bind_count = -1;

  f3d_test_check_null(&test, "interactor_get_binds_for_group(NULL group)",
    f3d_interactor_get_binds_for_group(interactor, NULL, &bind_count));

  f3d_test_check_null(&test, "interactor_get_binds_for_group(NULL count)",
    f3d_interactor_get_binds_for_group(interactor, "Test", NULL));

  f3d_interaction_bind_t* binds = f3d_interactor_get_binds_for_group(interactor, "Test", &bind_count);

  f3d_test_check_ptr(&test, "interactor_get_binds_for_group(valid)", binds);

  f3d_test_check(&test, "interactor_get_binds_for_group(non-empty)", bind_count > 0);

  if (binds)
  {
    f3d_interactor_free_bind_array(binds);
  }

  bind_count = -1;

  f3d_test_check_null(&test, "interactor_get_binds_for_group(invalid)", 
    f3d_interactor_get_binds_for_group(interactor, "InvalidGroup", &bind_count));

  f3d_test_check_int(&test, "interactor_get_binds_for_group(invalid count)", bind_count, 0);

  // f3d_interactor_get_binds tests
  bind_count = -1;

  f3d_test_check_null(&test, "interactor_get_binds(NULL interactor)", f3d_interactor_get_binds(NULL, &bind_count));

  f3d_test_check_int(&test, "interactor_get_binds(NULL interactor count)", bind_count, 0);

  bind_count = -1;

  f3d_test_check_null(&test, "interactor_get_binds(NULL count)", f3d_interactor_get_binds(interactor, NULL));

  binds = f3d_interactor_get_binds(interactor, &bind_count);

  f3d_test_check_ptr(&test, "interactor_get_binds(valid)", binds);

  f3d_test_check(&test, "interactor_get_binds(non-empty)", bind_count > 0);

  if (binds)
  {
    f3d_interactor_free_bind_array(binds);
  }

  // f3d_interactor_get_binding_documentation tests
  f3d_binding_documentation_t doc = { 0 };

  f3d_interactor_get_binding_documentation(NULL, &custom_bind, &doc);
  f3d_test_check(&test, "interactor_get_binding_documentation(NULL interactor)", 1);

  f3d_interactor_get_binding_documentation(interactor, NULL, &doc);
  f3d_test_check(&test, "interactor_get_binding_documentation(NULL bind)", 1);

  f3d_interactor_get_binding_documentation(interactor, &custom_bind, NULL);
  f3d_test_check(&test, "interactor_get_binding_documentation(NULL doc)", 1);

  memset(&doc, 0xFF, sizeof(doc));
  f3d_interactor_get_binding_documentation(interactor, &custom_bind, &doc);
  f3d_test_check(&test, "interactor_get_binding_documentation(valid, empty since no callback)",
    doc.doc[0] == '\0' && doc.value[0] == '\0');

  // f3d_interactor_get_binding_type tests
  f3d_test_check_int(&test, "interactor_get_binding_type(NULL interactor)",
    f3d_interactor_get_binding_type(NULL, &custom_bind), F3D_INTERACTOR_BINDING_OTHER);

  f3d_test_check_int(&test, "interactor_get_binding_type(NULL bind)",
    f3d_interactor_get_binding_type(interactor, NULL), F3D_INTERACTOR_BINDING_OTHER);

  f3d_test_check(&test, "interactor_get_binding_type(valid)",
    f3d_interactor_get_binding_type(interactor, &custom_bind) == F3D_INTERACTOR_BINDING_TOGGLE);

  // f3d_interactor_trigger_notification tests
  f3d_interactor_trigger_notification(NULL, "Description", "Value", 1.0);
  f3d_test_check(&test, "interactor_trigger_notification(NULL interactor)", 1);

  f3d_interactor_trigger_notification(interactor, NULL, "Value", 1.0);
  f3d_test_check(&test, "interactor_trigger_notification(NULL desc)", 1);

  f3d_interactor_trigger_notification(interactor, "Description", NULL, 1.0);
  f3d_test_check(&test, "interactor_trigger_notification(NULL value)", 1);

  f3d_interactor_trigger_notification(interactor, "Description", "Value", 1.0);
  f3d_test_check(&test, "interactor_trigger_notification(valid)", 1);

  // f3d_interactor_free_string_array tests
  f3d_interactor_free_string_array(NULL, 0);
  f3d_test_check(&test, "interactor_free_string_array(NULL)", 1);

  // f3d_interactor_free_bind_array tests
  f3d_interactor_free_bind_array(NULL);
  f3d_test_check(&test, "interactor_free_bind_array(NULL)", 1);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}