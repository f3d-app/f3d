#include <camera_c_api.h>
#include <engine_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

int test_camera()
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

  f3d_camera_t* camera = f3d_window_get_camera(window);
  if (!camera)
  {
    puts("[ERROR] Failed to get camera");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_point3_t pos = { 1.0, 2.0, 3.0 };
  f3d_camera_set_position(camera, pos);
  f3d_point3_t get_pos;
  f3d_camera_get_position(camera, get_pos);

  f3d_point3_t focal = { 0.0, 0.0, 0.0 };
  f3d_camera_set_focal_point(camera, focal);
  f3d_point3_t get_focal;
  f3d_camera_get_focal_point(camera, get_focal);

  f3d_vector3_t view_up = { 0.0, 1.0, 0.0 };
  f3d_camera_set_view_up(camera, view_up);
  f3d_vector3_t get_view_up;
  f3d_camera_get_view_up(camera, get_view_up);

  f3d_camera_set_view_angle(camera, 30.0);
  f3d_angle_deg_t angle = f3d_camera_get_view_angle(camera);
  (void)angle;

  f3d_camera_state_t state = { 0 };
  state.position[0] = 5.0;
  state.position[1] = 5.0;
  state.position[2] = 5.0;
  state.focal_point[0] = 0.0;
  state.focal_point[1] = 0.0;
  state.focal_point[2] = 0.0;
  state.view_up[0] = 0.0;
  state.view_up[1] = 1.0;
  state.view_up[2] = 0.0;
  state.view_angle = 45.0;
  f3d_camera_set_state(camera, &state);
  f3d_camera_state_t get_state;
  f3d_camera_get_state(camera, &get_state);

  f3d_camera_dolly(camera, 1.5);
  f3d_camera_pan(camera, 0.1, 0.2, 0.3);
  f3d_camera_zoom(camera, 0.9);
  f3d_camera_roll(camera, 10.0);
  f3d_camera_azimuth(camera, 15.0);
  f3d_camera_yaw(camera, 20.0);
  f3d_camera_elevation(camera, 25.0);
  f3d_camera_pitch(camera, 30.0);

  f3d_camera_set_current_as_default(camera);
  f3d_camera_reset_to_default(camera);
  f3d_camera_reset_to_bounds(camera, 0.9);

  f3d_engine_delete(engine);
  return 0;
}
