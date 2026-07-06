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

  f3d_point3_t pos = { 0.0, 0.0, 10.0 };
  f3d_camera_set_position(camera, pos);
  f3d_point3_t get_pos;
  f3d_camera_get_position(camera, get_pos);
  f3d_test_check_double(&test, "position X matches", get_pos[0], pos[0], 1e-9);
  f3d_test_check_double(&test, "position Y matches", get_pos[1], pos[1], 1e-9);
  f3d_test_check_double(&test, "position Z matches", get_pos[2], pos[2], 1e-9);

  f3d_point3_t focal = { 0.0, 0.0, -1.0 };
  f3d_camera_set_focal_point(camera, focal);
  f3d_point3_t get_focal;
  f3d_camera_get_focal_point(camera, get_focal);
  f3d_test_check_double(&test, "focal point X matches", get_focal[0], focal[0], 1e-9);
  f3d_test_check_double(&test, "focal point Y matches", get_focal[1], focal[1], 1e-9);
  f3d_test_check_double(&test, "focal point Z matches", get_focal[2], focal[2], 1e-9);

  f3d_vector3_t view_up = { 1.0, 0.0, 0.0 };
  f3d_camera_set_view_up(camera, view_up);
  f3d_vector3_t get_view_up;
  f3d_camera_get_view_up(camera, get_view_up);
  f3d_test_check_double(&test, "view up X matches", get_view_up[0], view_up[0], 1e-9);
  f3d_test_check_double(&test, "view up Y matches", get_view_up[1], view_up[1], 1e-9);
  f3d_test_check_double(&test, "view up Z matches", get_view_up[2], view_up[2], 1e-9);

  f3d_camera_set_view_angle(camera, 20.0);
  f3d_angle_deg_t angle = f3d_camera_get_view_angle(camera);
  f3d_test_check_double(&test, "view angle matches", angle, 20.0, 1e-9);

  f3d_camera_state_t state = { 0 };
  state.position[0] = 0.0;
  state.position[1] = 0.0;
  state.position[2] = 10.0;
  state.focal_point[0] = 0.0;
  state.focal_point[1] = 0.0;
  state.focal_point[2] = -1.0;
  state.view_up[0] = 1.0;
  state.view_up[1] = 0.0;
  state.view_up[2] = 0.0;
  state.view_angle = 45.0;
  f3d_camera_set_state(camera, &state);
  f3d_camera_state_t get_state;
  f3d_camera_get_state(camera, &get_state);

  f3d_test_check_double(
    &test, "state position X matches", get_state.position[0], state.position[0], 1e-9);
  f3d_test_check_double(
    &test, "state position Y matches", get_state.position[1], state.position[1], 1e-9);
  f3d_test_check_double(
    &test, "state position Z matches", get_state.position[2], state.position[2], 1e-9);
  f3d_test_check_double(
    &test, "state focal point X matches", get_state.focal_point[0], state.focal_point[0], 1e-9);
  f3d_test_check_double(
    &test, "state focal point Y matches", get_state.focal_point[1], state.focal_point[1], 1e-9);
  f3d_test_check_double(
    &test, "state focal point Z matches", get_state.focal_point[2], state.focal_point[2], 1e-9);
  f3d_test_check_double(
    &test, "state view up X matches", get_state.view_up[0], state.view_up[0], 1e-9);
  f3d_test_check_double(
    &test, "state view up Y matches", get_state.view_up[1], state.view_up[1], 1e-9);
  f3d_test_check_double(
    &test, "state view up Z matches", get_state.view_up[2], state.view_up[2], 1e-9);
  f3d_test_check_double(
    &test, "state view angle matches", get_state.view_angle, state.view_angle, 1e-9);

  f3d_camera_dolly(camera, 1.5);
  f3d_camera_pan(camera, 0.1, 0.2, 0.3);
  f3d_camera_zoom(camera, 0.9);
  f3d_camera_roll(camera, 10.0);
  f3d_camera_azimuth(camera, 15.0);
  f3d_camera_yaw(camera, 20.0);
  f3d_camera_elevation(camera, 25.0);
  f3d_camera_pitch(camera, 30.0);
  f3d_test_check(&test, "camera still valid after transforms", camera != NULL);

  f3d_camera_set_current_as_default(camera);
  f3d_camera_reset_to_default(camera);
  f3d_camera_reset_to_bounds(camera, 0.9);
  f3d_test_check(&test, "camera still valid after reset calls", camera != NULL);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
