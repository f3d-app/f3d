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

  //f3d_window_type_t tests
  f3d_test_check_int(&test, "window_get_type(NULL) returns UNKNOWN", f3d_window_get_type(NULL), F3D_WINDOW_UNKNOWN);
  f3d_test_check(&test, "window_get_type returns valid type", f3d_window_get_type(window) != F3D_WINDOW_UNKNOWN);
  
  //f3d_window_is_offscreen tests
  f3d_test_check_int(&test, "window_is_offscreen(NULL) returns 0", f3d_window_is_offscreen(NULL), 0);
  f3d_test_check_int( &test, "window_is_offscreen(engine window)", f3d_window_is_offscreen(window), 1);

  //f3d_window_get_camera tests
  f3d_test_check_null(&test, "window_get_camera(NULL) returns NULL", f3d_window_get_camera(NULL));
  f3d_test_check_ptr(&test, "window_get_camera(engine window)", f3d_window_get_camera(window));

  //f3d_window_render tests
  f3d_test_check_int(&test, "window_render(NULL) returns 0", f3d_window_render(NULL), 0);
  f3d_test_check_int(&test, "window_render(engine window)", f3d_window_render(window), 1);

  //f3d_window_render_to_image tests
  f3d_image_t* image = f3d_window_render_to_image(window, 0);
  f3d_test_check_ptr(&test, "window_render_to_image(engine window)", image);
  f3d_image_delete(image);

  image = f3d_window_render_to_image(window, 1);
  f3d_test_check_ptr(&test, "window_render_to_image(no background)", image);
  f3d_image_delete(image);
  
  //f3d_window_set_size tests
  f3d_window_set_size(NULL, 800, 600);
  f3d_test_check(&test, "window_set_size(NULL) does not crash", 1);

  f3d_window_set_size(window, 800, 600);
  f3d_test_check_int(&test, "window_get_width after set_size", f3d_window_get_width(window), 800);
  f3d_test_check_int(&test, "window_get_height after set_size", f3d_window_get_height(window), 600);

  //f3d_window_get_width and f3d_window_get_height tests
  f3d_test_check_int(&test, "window_get_width(NULL) returns 0", f3d_window_get_width(NULL), 0);
  f3d_test_check_int(&test, "window_get_height(NULL) returns 0", f3d_window_get_height(NULL), 0);

  //f3d_window_set_position tests (no way to verify position, just check for crashes)
  f3d_window_set_position(NULL, 100, 100);
  f3d_test_check(&test, "window_set_position(NULL) does not crash", 1);

  f3d_window_set_position(window, 100, 100);
  f3d_test_check(&test, "window_set_position(engine window)", 1);

  //f3d_window_set_icon tests (no way to verify icon, just check for crashes)
  unsigned char icon_data[] = { 0xFF, 0xFF, 0xFF, 0xFF };

  f3d_window_set_icon(NULL, icon_data, sizeof(icon_data));
  f3d_test_check(&test, "window_set_icon(NULL window) does not crash", 1);

  f3d_window_set_icon(window, NULL, 0);
  f3d_test_check(&test, "window_set_icon(NULL icon) does not crash", 1);
  
  f3d_window_set_icon(window, icon_data, sizeof(icon_data));
  f3d_test_check(&test, "window_set_icon(valid)", 1);

  //f3d_window_set_window_name tests (no way to verify name, just check for crashes)
  f3d_window_set_window_name(NULL, "Test Window");
  f3d_test_check(&test, "window_set_window_name(NULL window) does not crash", 1);

  f3d_window_set_window_name(window, NULL);
  f3d_test_check(&test, "window_set_window_name(NULL name) does not crash", 1);

  f3d_window_set_window_name(window, "Test Window");
  f3d_test_check(&test, "window_set_window_name(valid)", 1);

  // f3d_window_get_world_from_display and f3d_window_get_display_from_world tests
  f3d_point3_t display_point = { 400.0, 300.0, 0.0 };
  f3d_point3_t world_point = { 0.0, 0.0, 0.0 };
  f3d_point3_t display_out = { 0.0, 0.0, 0.0 };

  f3d_window_get_world_from_display(NULL, display_point, world_point);
  f3d_test_check(&test, "window_get_world_from_display(NULL window)", 1);

  f3d_window_get_world_from_display(window, NULL, world_point);
  f3d_test_check(&test, "window_get_world_from_display(NULL display_point)", 1);

  f3d_window_get_world_from_display(window, display_point, NULL);
  f3d_test_check(&test, "window_get_world_from_display(NULL world_point)", 1);

  f3d_window_get_display_from_world(NULL, world_point, display_out);
  f3d_test_check(&test, "window_get_display_from_world(NULL window)", 1);

  f3d_window_get_display_from_world(window, NULL, display_out);
  f3d_test_check(&test, "window_get_display_from_world(NULL world_point)", 1);

  f3d_window_get_display_from_world(window, world_point, NULL);
  f3d_test_check(&test, "window_get_display_from_world(NULL display_point)", 1);

  // Round-trip conversion
  f3d_window_get_world_from_display(window, display_point, world_point);
  f3d_window_get_display_from_world(window, world_point, display_out);

  f3d_test_check_vec3(&test, "display -> world -> display", display_out, display_point);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
