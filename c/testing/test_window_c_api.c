#include <engine_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

int test_window_c_api()
{
  int failed = 0;

  f3d_engine_t* engine = f3d_engine_create(1);
  if (engine == NULL)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (window == NULL)
  {
    puts("[ERROR] Failed to get window");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_window_type_t type = f3d_window_get_type(window);
  if (type == F3D_WINDOW_UNKNOWN)
  {
    puts("[ERROR] Window type should not be UNKNOWN");
    failed++;
  }

  int offscreen = f3d_window_is_offscreen(window);
  if (!offscreen)
  {
    puts("[ERROR] Window should be offscreen");
    failed++;
  }

  const f3d_camera_t* camera = f3d_window_get_camera(window);
  if (camera == NULL)
  {
    puts("[ERROR] Window should have a camera");
    failed++;
  }

  f3d_window_set_size(window, 800, 600);

  int width = f3d_window_get_width(window);
  int height = f3d_window_get_height(window);

  if (width != 800 || height != 600)
  {
    puts("[ERROR] Window size should be 800x600");
    failed++;
  }

  f3d_engine_delete(engine);

  return failed;
}
