#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

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

  f3d_interactor_toggle_animation(interactor);
  f3d_interactor_start_animation(interactor);
  int playing = f3d_interactor_is_playing_animation(interactor);
  (void)playing;
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

  f3d_engine_delete(engine);
  return 0;
}
