#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <window_c_api.h>

#include <stdio.h>
#include <string.h>

int test_interactor_c_api()
{
  int failed = 0;

  f3d_engine_t* engine = f3d_engine_create(1);
  if (engine == NULL)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  if (interactor == NULL)
  {
    puts("[ERROR] Failed to get interactor");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (window != NULL)
  {
    f3d_window_render(window);
  }

  f3d_interactor_toggle_animation(interactor);
  f3d_interactor_start_animation(interactor);
  f3d_interactor_stop_animation(interactor);

  int playing = f3d_interactor_is_playing_animation(interactor);
  if (playing != 0)
  {
    puts("[ERROR] Animation should not be playing");
    failed++;
  }

  f3d_interactor_enable_camera_movement(interactor);
  f3d_interactor_disable_camera_movement(interactor);

  f3d_interactor_trigger_mouse_button(
    interactor, F3D_INTERACTOR_INPUT_PRESS, F3D_INTERACTOR_MOUSE_LEFT);
  f3d_interactor_trigger_mouse_position(interactor, 100.0, 100.0);
  f3d_interactor_trigger_mouse_wheel(interactor, F3D_INTERACTOR_WHEEL_FORWARD);
  f3d_interactor_trigger_keyboard_key(interactor, F3D_INTERACTOR_INPUT_PRESS, "A");

  f3d_interactor_init_commands(interactor);
  f3d_interactor_init_bindings(interactor);

  f3d_engine_delete(engine);

  return failed;
}
