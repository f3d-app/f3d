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
  f3d_test_check_vec3(&test, "position matches", get_pos, pos, 1e-9);

  f3d_point3_t focal = { 0.0, 0.0, -1.0 };
  f3d_camera_set_focal_point(camera, focal);
  f3d_point3_t get_focal;
  f3d_camera_get_focal_point(camera, get_focal);
  f3d_test_check_vec3(&test, "focal point matches", get_focal, focal, 1e-9);

  f3d_vector3_t view_up = { 1.0, 0.0, 0.0 };
  f3d_camera_set_view_up(camera, view_up);
  f3d_vector3_t get_view_up;
  f3d_camera_get_view_up(camera, get_view_up);
  f3d_test_check_vec3(&test, "view up matches", get_view_up, view_up, 1e-9);

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

  f3d_test_check_vec3(&test, "state position matches", get_state.position, state.position, 1e-9);
  f3d_test_check_vec3(&test, "state focal point matches", get_state.focal_point, state.focal_point, 1e-9);
  f3d_test_check_vec3(&test, "state view up matches", get_state.view_up, state.view_up, 1e-9);
  f3d_test_check_double(&test, "state view angle matches", get_state.view_angle, state.view_angle, 1e-9);

  f3d_camera_state_t state_before_transforms;
  f3d_camera_get_state(camera, &state_before_transforms);

  f3d_camera_dolly(camera, 1.5);
  f3d_camera_pan(camera, 0.1, 0.2, 0.3);
  f3d_camera_zoom(camera, 0.9);
  f3d_camera_roll(camera, 10.0);
  f3d_camera_azimuth(camera, 15.0);
  f3d_camera_yaw(camera, 20.0);
  f3d_camera_elevation(camera, 25.0);
  f3d_camera_pitch(camera, 30.0);

  f3d_camera_state_t state_after_transforms;
  f3d_camera_get_state(camera, &state_after_transforms);
  f3d_test_check(&test, "transform bindings reach the camera and change its state",
    state_after_transforms.position[0] != state_before_transforms.position[0] ||
    state_after_transforms.position[1] != state_before_transforms.position[1] ||
    state_after_transforms.position[2] != state_before_transforms.position[2] ||
    state_after_transforms.focal_point[0] != state_before_transforms.focal_point[0] ||
    state_after_transforms.focal_point[1] != state_before_transforms.focal_point[1] ||
    state_after_transforms.focal_point[2] != state_before_transforms.focal_point[2] ||
    state_after_transforms.view_up[0] != state_before_transforms.view_up[0] ||
    state_after_transforms.view_up[1] != state_before_transforms.view_up[1] ||
    state_after_transforms.view_up[2] != state_before_transforms.view_up[2] ||
    state_after_transforms.view_angle != state_before_transforms.view_angle);

  f3d_camera_set_current_as_default(camera);
  f3d_camera_state_t default_state;
  f3d_camera_get_state(camera, &default_state);

  f3d_camera_dolly(camera, 2.0); // mutate away from default
  f3d_camera_reset_to_default(camera);
  f3d_camera_state_t restored_state;
  f3d_camera_get_state(camera, &restored_state);
  f3d_test_check_vec3(&test, "reset_to_default restores position",
  restored_state.position, default_state.position, 1e-9);

  f3d_camera_reset_to_bounds(camera, 0.9);
  // smoke call only: engine has no scene/actors loaded, so there are no bounds
  // to reset to and no state change is guaranteed here

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
