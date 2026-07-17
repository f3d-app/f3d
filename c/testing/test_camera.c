#include "pseudo_unit_test.h"
#include <camera_c_api.h>
#include <engine_c_api.h>
#include <window_c_api.h>

int test_camera()
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

  f3d_camera_t* camera = f3d_window_get_camera(window);
  f3d_test_check(&test, "camera retrieved", camera != NULL);
  if (!camera)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  // f3d_camera_set_position and f3d_camera_get_position tests
  f3d_point3_t position = { 1.0, 2.0, 3.0 };
  f3d_point3_t position_out = { 0.0, 0.0, 0.0 };

  f3d_camera_set_position(NULL, position);
  f3d_test_check(&test, "camera_set_position(NULL camera)", 1);

  f3d_camera_set_position(camera, NULL);
  f3d_test_check(&test, "camera_set_position(NULL position)", 1);

  f3d_camera_get_position(NULL, position_out);
  f3d_test_check(&test, "camera_get_position(NULL camera)", 1);

  f3d_camera_get_position(camera, NULL);
  f3d_test_check(&test, "camera_get_position(NULL position)", 1);

  f3d_camera_set_position(camera, position);
  f3d_camera_get_position(camera, position_out);
  f3d_test_check_vec3(&test, "camera_get_position after set_position", position_out, position);

  // f3d_camera_set_focal_point and f3d_camera_get_focal_point tests
  f3d_point3_t focal_point = { 4.0, 5.0, 6.0 };
  f3d_point3_t focal_point_out = { 0.0, 0.0, 0.0 };

  f3d_camera_set_focal_point(NULL, focal_point);
  f3d_test_check(&test, "camera_set_focal_point(NULL camera)", 1);

  f3d_camera_set_focal_point(camera, NULL);
  f3d_test_check(&test, "camera_set_focal_point(NULL focal_point)", 1);

  f3d_camera_get_focal_point(NULL, focal_point_out);
  f3d_test_check(&test, "camera_get_focal_point(NULL camera)", 1);

  f3d_camera_get_focal_point(camera, NULL);
  f3d_test_check(&test, "camera_get_focal_point(NULL focal_point)", 1);

  f3d_camera_set_focal_point(camera, focal_point);
  f3d_camera_get_focal_point(camera, focal_point_out);

  f3d_test_check_vec3(&test, "camera_get_focal_point after set_focal_point", focal_point_out, focal_point);

  // f3d_camera_set_view_up and f3d_camera_get_view_up tests
  f3d_vector3_t view_up = {0.7071067811865475, -0.7071067811865475, 0.0};
  f3d_vector3_t view_up_out = { 0.0, 0.0, 0.0 };

  f3d_camera_set_view_up(NULL, view_up);
  f3d_test_check(&test, "camera_set_view_up(NULL camera)", 1);

  f3d_camera_set_view_up(camera, NULL);
  f3d_test_check(&test, "camera_set_view_up(NULL view_up)", 1);

  f3d_camera_get_view_up(NULL, view_up_out);
  f3d_test_check(&test, "camera_get_view_up(NULL camera)", 1);

  f3d_camera_get_view_up(camera, NULL);
  f3d_test_check(&test, "camera_get_view_up(NULL view_up)", 1);

  f3d_camera_set_view_up(camera, view_up);
  f3d_camera_get_view_up(camera, view_up_out);

  f3d_test_check_vec3(&test, "camera_get_view_up after set_view_up", view_up_out, view_up);

  // f3d_camera_set_view_angle and f3d_camera_get_view_angle tests
  f3d_test_check_double(&test, "camera_get_view_angle(NULL) returns 0.0", f3d_camera_get_view_angle(NULL), 0.0);

  f3d_camera_set_view_angle(NULL, 45.0);
  f3d_test_check(&test, "camera_set_view_angle(NULL camera)", 1);

  f3d_camera_set_view_angle(camera, 45.0);
  f3d_test_check_double(&test, "camera_get_view_angle after set_view_angle", f3d_camera_get_view_angle(camera), 45.0);

  // f3d_camera_set_state and f3d_camera_get_state tests
  f3d_camera_state_t state = {
    .position = { 1.0, 2.0, 3.0 },
    .focal_point = { 4.0, 5.0, 6.0 },
    .view_up = {0.7071067811865475, -0.7071067811865475, 0.0},
    .view_angle = 45.0
  };

  f3d_camera_state_t state_out = { 0 };

  f3d_camera_set_state(NULL, &state);
  f3d_test_check(&test, "camera_set_state(NULL camera)", 1);

  f3d_camera_set_state(camera, NULL);
  f3d_test_check(&test, "camera_set_state(NULL state)", 1);

  f3d_camera_get_state(NULL, &state_out);
  f3d_test_check(&test, "camera_get_state(NULL camera)", 1);

  f3d_camera_get_state(camera, NULL);
  f3d_test_check(&test, "camera_get_state(NULL state)", 1);

  f3d_camera_set_state(camera, &state);
  f3d_camera_get_state(camera, &state_out);

  f3d_test_check_vec3(&test, "camera_state.position", state_out.position, state.position);
  f3d_test_check_vec3(&test, "camera_state.focal_point", state_out.focal_point, state.focal_point);
  f3d_test_check_vec3(&test, "camera_state.view_up", state_out.view_up, state.view_up);
  f3d_test_check_double(&test, "camera_state.view_angle", state_out.view_angle, state.view_angle);

  // Camera movement operation tests
  f3d_camera_dolly(NULL, 2.0);
  f3d_test_check(&test, "camera_dolly(NULL camera)", 1);

  f3d_camera_dolly(camera, 2.0);
  f3d_test_check(&test, "camera_dolly(valid)", 1);

  f3d_camera_pan(NULL, 1.0, 2.0, 3.0);
  f3d_test_check(&test, "camera_pan(NULL camera)", 1);

  f3d_camera_pan(camera, 1.0, 2.0, 3.0);
  f3d_test_check(&test, "camera_pan(valid)", 1);

  f3d_camera_zoom(NULL, 2.0);
  f3d_test_check(&test, "camera_zoom(NULL camera)", 1);

  f3d_camera_zoom(camera, 2.0);
  f3d_test_check(&test, "camera_zoom(valid)", 1);

  f3d_camera_roll(NULL, 45.0);
  f3d_test_check(&test, "camera_roll(NULL camera)", 1);

  f3d_camera_roll(camera, 45.0);
  f3d_test_check(&test, "camera_roll(valid)", 1);

  f3d_camera_azimuth(NULL, 45.0);
  f3d_test_check(&test, "camera_azimuth(NULL camera)", 1);

  f3d_camera_azimuth(camera, 45.0);
  f3d_test_check(&test, "camera_azimuth(valid)", 1);

  f3d_camera_yaw(NULL, 45.0);
  f3d_test_check(&test, "camera_yaw(NULL camera)", 1);

  f3d_camera_yaw(camera, 45.0);
  f3d_test_check(&test, "camera_yaw(valid)", 1);

  f3d_camera_elevation(NULL, 45.0);
  f3d_test_check(&test, "camera_elevation(NULL camera)", 1);

  f3d_camera_elevation(camera, 45.0);
  f3d_test_check(&test, "camera_elevation(valid)", 1);

  f3d_camera_pitch(NULL, 45.0);
  f3d_test_check(&test, "camera_pitch(NULL camera)", 1);

  f3d_camera_pitch(camera, 45.0);
  f3d_test_check(&test, "camera_pitch(valid)", 1);

  // f3d_camera_set_current_as_default tests
  f3d_camera_set_current_as_default(NULL);
  f3d_test_check(&test, "camera_set_current_as_default(NULL camera)", 1);

  f3d_camera_set_current_as_default(camera);
  f3d_test_check(&test, "camera_set_current_as_default(valid)", 1);

  // f3d_camera_reset_to_default tests
  f3d_camera_reset_to_default(NULL);
  f3d_test_check(&test, "camera_reset_to_default(NULL camera)", 1);

  f3d_camera_set_position(camera, position);
  f3d_camera_set_current_as_default(camera);

  f3d_point3_t new_position = { 4.0, 5.0, 6.0 };
  f3d_camera_set_position(camera, new_position);

  f3d_camera_reset_to_default(camera);

  f3d_camera_get_position(camera, position_out);
  f3d_test_check_vec3(&test, "camera_reset_to_default restores default position", position_out, position);

  // f3d_camera_reset_to_bounds tests
  f3d_camera_reset_to_bounds(NULL, 1.0);
  f3d_test_check(&test, "camera_reset_to_bounds(NULL camera)", 1);

  f3d_camera_reset_to_bounds(camera, 1.0);
  f3d_test_check(&test, "camera_reset_to_bounds(valid)", 1);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
