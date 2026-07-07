#include "pseudo_unit_test.h"

#include <engine_c_api.h>
#include <image_c_api.h>
#include <window_c_api.h>

int test_window()
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
  if (!window)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  f3d_window_type_t type = f3d_window_get_type(window);
  f3d_test_check(&test, "window type is a real backend, not NONE/UNKNOWN",
    type != F3D_WINDOW_NONE && type != F3D_WINDOW_UNKNOWN);

  int offscreen = f3d_window_is_offscreen(window);
  f3d_test_check_int(&test, "window is offscreen", offscreen, 1);

  f3d_camera_t* camera = f3d_window_get_camera(window);
  f3d_test_check(&test, "camera retrieved from window", camera != NULL);

  int render_ret = f3d_window_render(window);
  f3d_test_check_int(&test, "render succeeds on a valid offscreen window", render_ret, 1);

  f3d_image_t* img = f3d_window_render_to_image(window, 0);
  f3d_test_check(&test, "render_to_image returns a non-null image", img != NULL);
  if (img)
  {
    f3d_image_delete(img);
  }

  f3d_window_set_size(window, 800, 600);
  int width = f3d_window_get_width(window);
  int height = f3d_window_get_height(window);
  f3d_test_check_int(&test, "window width matches after set_size", width, 800);
  f3d_test_check_int(&test, "window height matches after set_size", height, 600);

  /* no getter exists to verify these took effect, so they remain smoke calls */
  f3d_window_set_position(window, 100, 100);

  unsigned char icon_data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  f3d_window_set_icon(window, icon_data, sizeof(icon_data));

  f3d_window_set_window_name(window, "Test Window");

  f3d_point3_t display_point = { 400.0, 300.0, 0.0 };
  f3d_point3_t world_point;
  f3d_window_get_world_from_display(window, display_point, world_point);

  f3d_point3_t display_roundtrip;
  f3d_window_get_display_from_world(window, world_point, display_roundtrip);
  f3d_test_check_double(&test, "display->world->display roundtrip X matches",
    display_roundtrip[0], display_point[0], 1e-6);
  f3d_test_check_double(&test, "display->world->display roundtrip Y matches",
    display_roundtrip[1], display_point[1], 1e-6);
  f3d_test_check_double(&test, "display->world->display roundtrip Z matches",
    display_roundtrip[2], display_point[2], 1e-6);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
