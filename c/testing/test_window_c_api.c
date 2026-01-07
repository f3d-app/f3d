#include <engine_c_api.h>
#include <image_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

int test_window_c_api()
{
  f3d_engine_t* engine = f3d_engine_create(1);
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (!window)
  {
    puts("[ERROR] Failed to get window");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_window_type_t type = f3d_window_get_type(window);
  (void)type;

  int offscreen = f3d_window_is_offscreen(window);
  (void)offscreen;

  f3d_camera_t* camera = f3d_window_get_camera(window);
  (void)camera;

  f3d_window_render(window);

  f3d_image_t* img = f3d_window_render_to_image(window, 0);
  if (img)
  {
    f3d_image_delete(img);
  }

  f3d_window_set_size(window, 800, 600);
  int width = f3d_window_get_width(window);
  (void)width;
  int height = f3d_window_get_height(window);
  (void)height;

  f3d_window_set_position(window, 100, 100);

  unsigned char icon_data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  f3d_window_set_icon(window, icon_data, sizeof(icon_data));

  f3d_window_set_window_name(window, "Test Window");

  f3d_point3_t display_point = { 400.0, 300.0, 0.0 };
  f3d_point3_t world_point;
  f3d_window_get_world_from_display(window, display_point, world_point);

  f3d_point3_t test_world = { 0.0, 0.0, 0.0 };
  f3d_point3_t display_out;
  f3d_window_get_display_from_world(window, test_world, display_out);

  f3d_engine_delete(engine);
  return 0;
}
