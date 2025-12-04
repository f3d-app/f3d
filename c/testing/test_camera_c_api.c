#include <camera_c_api.h>
#include <engine_c_api.h>
#include <types_c_api.h>
#include <window_c_api.h>

#include <math.h>
#include <stdio.h>

static int double_equal(double a, double b, double tolerance)
{
  return fabs(a - b) < tolerance;
}

static int point3_equal(const f3d_point3_t a, const f3d_point3_t b, double tolerance)
{
  return double_equal(a[0], b[0], tolerance) && double_equal(a[1], b[1], tolerance) &&
    double_equal(a[2], b[2], tolerance);
}

int test_camera_c_api()
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

  f3d_camera_t* camera = f3d_window_get_camera(window);
  if (camera == NULL)
  {
    puts("[ERROR] Failed to get camera");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_point3_t pos = { 1.0, 2.0, 3.0 };
  f3d_point3_t focal = { 1.0, 22.0, 3.0 };
  f3d_vector3_t view_up = { 0.0, 0.0, 1.0 };
  f3d_angle_deg_t angle = 32.0;

  f3d_camera_set_position(camera, pos);
  f3d_camera_set_focal_point(camera, focal);
  f3d_camera_set_view_up(camera, view_up);
  f3d_camera_set_view_angle(camera, angle);

  f3d_point3_t got_pos;
  f3d_camera_get_position(camera, got_pos);
  if (!point3_equal(pos, got_pos, 1e-6))
  {
    puts("[ERROR] Camera position mismatch");
    failed++;
  }

  f3d_point3_t got_focal;
  f3d_camera_get_focal_point(camera, got_focal);
  if (!point3_equal(focal, got_focal, 1e-6))
  {
    puts("[ERROR] Camera focal point mismatch");
    failed++;
  }

  f3d_vector3_t got_view_up;
  f3d_camera_get_view_up(camera, got_view_up);
  if (!point3_equal(view_up, got_view_up, 1e-6))
  {
    puts("[ERROR] Camera view up mismatch");
    failed++;
  }

  f3d_angle_deg_t got_angle = f3d_camera_get_view_angle(camera);
  if (!double_equal(angle, got_angle, 1e-6))
  {
    puts("[ERROR] Camera view angle mismatch");
    failed++;
  }

  f3d_camera_state_t state = { 0 };
  f3d_camera_get_state(camera, &state);
  if (!point3_equal(pos, state.position, 1e-6))
  {
    puts("[ERROR] Camera state position mismatch");
    failed++;
  }

  f3d_camera_state_t new_state = { 0 };
  new_state.position[0] = 5.0;
  new_state.position[1] = 6.0;
  new_state.position[2] = 7.0;
  new_state.focal_point[0] = 0.0;
  new_state.focal_point[1] = 0.0;
  new_state.focal_point[2] = 0.0;
  new_state.view_up[0] = 0.0;
  new_state.view_up[1] = 1.0;
  new_state.view_up[2] = 0.0;
  new_state.view_angle = 45.0;

  f3d_camera_set_state(camera, &new_state);
  f3d_camera_get_position(camera, got_pos);
  if (!point3_equal(new_state.position, got_pos, 1e-6))
  {
    puts("[ERROR] Camera state set failed");
    failed++;
  }

  f3d_camera_dolly(camera, 2.0);
  f3d_camera_roll(camera, 30.0);
  f3d_camera_azimuth(camera, 30.0);
  f3d_camera_yaw(camera, 15.0);
  f3d_camera_elevation(camera, 20.0);
  f3d_camera_pitch(camera, 10.0);

  f3d_camera_set_current_as_default(camera);
  f3d_camera_reset_to_default(camera);
  f3d_camera_reset_to_bounds(camera, 0.9);

  f3d_engine_delete(engine);

  return failed;
}
