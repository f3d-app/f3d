#include "camera_c_api.h"
#include "camera.h"

//----------------------------------------------------------------------------
void f3d_camera_set_position(f3d_camera_t* camera, const f3d_point3_t pos)
{
  if (!camera || !pos)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  f3d::point3_t cpp_pos = { pos[0], pos[1], pos[2] };
  cpp_camera->setPosition(cpp_pos);
}

//----------------------------------------------------------------------------
void f3d_camera_get_position(const f3d_camera_t* camera, f3d_point3_t pos)
{
  if (!camera || !pos)
  {
    return;
  }

  const f3d::camera* cpp_camera = reinterpret_cast<const f3d::camera*>(camera);
  f3d::point3_t cpp_pos;
  cpp_camera->getPosition(cpp_pos);
  pos[0] = cpp_pos[0];
  pos[1] = cpp_pos[1];
  pos[2] = cpp_pos[2];
}

//----------------------------------------------------------------------------
void f3d_camera_set_focal_point(f3d_camera_t* camera, const f3d_point3_t focal_point)
{
  if (!camera || !focal_point)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  f3d::point3_t cpp_focal = { focal_point[0], focal_point[1], focal_point[2] };
  cpp_camera->setFocalPoint(cpp_focal);
}

//----------------------------------------------------------------------------
void f3d_camera_get_focal_point(const f3d_camera_t* camera, f3d_point3_t focal_point)
{
  if (!camera || !focal_point)
  {
    return;
  }

  const f3d::camera* cpp_camera = reinterpret_cast<const f3d::camera*>(camera);
  f3d::point3_t cpp_focal;
  cpp_camera->getFocalPoint(cpp_focal);
  focal_point[0] = cpp_focal[0];
  focal_point[1] = cpp_focal[1];
  focal_point[2] = cpp_focal[2];
}

//----------------------------------------------------------------------------
void f3d_camera_set_view_up(f3d_camera_t* camera, const f3d_vector3_t view_up)
{
  if (!camera || !view_up)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  f3d::vector3_t cpp_view_up = { view_up[0], view_up[1], view_up[2] };
  cpp_camera->setViewUp(cpp_view_up);
}

//----------------------------------------------------------------------------
void f3d_camera_get_view_up(const f3d_camera_t* camera, f3d_vector3_t view_up)
{
  if (!camera || !view_up)
  {
    return;
  }

  const f3d::camera* cpp_camera = reinterpret_cast<const f3d::camera*>(camera);
  f3d::vector3_t cpp_view_up;
  cpp_camera->getViewUp(cpp_view_up);
  view_up[0] = cpp_view_up[0];
  view_up[1] = cpp_view_up[1];
  view_up[2] = cpp_view_up[2];
}

//----------------------------------------------------------------------------
void f3d_camera_set_view_angle(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->setViewAngle(angle);
}

//----------------------------------------------------------------------------
f3d_angle_deg_t f3d_camera_get_view_angle(const f3d_camera_t* camera)
{
  if (!camera)
  {
    return 0.0;
  }

  const f3d::camera* cpp_camera = reinterpret_cast<const f3d::camera*>(camera);
  return cpp_camera->getViewAngle();
}

//----------------------------------------------------------------------------
void f3d_camera_set_state(f3d_camera_t* camera, const f3d_camera_state_t* state)
{
  if (!camera || !state)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  f3d::camera_state_t cpp_state;
  cpp_state.position = { state->position[0], state->position[1], state->position[2] };
  cpp_state.focalPoint = { state->focal_point[0], state->focal_point[1], state->focal_point[2] };
  cpp_state.viewUp = { state->view_up[0], state->view_up[1], state->view_up[2] };
  cpp_state.viewAngle = state->view_angle;
  cpp_camera->setState(cpp_state);
}

//----------------------------------------------------------------------------
void f3d_camera_get_state(const f3d_camera_t* camera, f3d_camera_state_t* state)
{
  if (!camera || !state)
  {
    return;
  }

  const f3d::camera* cpp_camera = reinterpret_cast<const f3d::camera*>(camera);
  f3d::camera_state_t cpp_state;
  cpp_camera->getState(cpp_state);
  state->position[0] = cpp_state.position[0];
  state->position[1] = cpp_state.position[1];
  state->position[2] = cpp_state.position[2];
  state->focal_point[0] = cpp_state.focalPoint[0];
  state->focal_point[1] = cpp_state.focalPoint[1];
  state->focal_point[2] = cpp_state.focalPoint[2];
  state->view_up[0] = cpp_state.viewUp[0];
  state->view_up[1] = cpp_state.viewUp[1];
  state->view_up[2] = cpp_state.viewUp[2];
  state->view_angle = cpp_state.viewAngle;
}

//----------------------------------------------------------------------------
void f3d_camera_dolly(f3d_camera_t* camera, double val)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->dolly(val);
}

//----------------------------------------------------------------------------
void f3d_camera_pan(f3d_camera_t* camera, double right, double up, double forward)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->pan(right, up, forward);
}

//----------------------------------------------------------------------------
void f3d_camera_zoom(f3d_camera_t* camera, double factor)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->zoom(factor);
}

//----------------------------------------------------------------------------
void f3d_camera_roll(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->roll(angle);
}

//----------------------------------------------------------------------------
void f3d_camera_azimuth(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->azimuth(angle);
}

//----------------------------------------------------------------------------
void f3d_camera_yaw(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->yaw(angle);
}

//----------------------------------------------------------------------------
void f3d_camera_elevation(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->elevation(angle);
}

//----------------------------------------------------------------------------
void f3d_camera_pitch(f3d_camera_t* camera, f3d_angle_deg_t angle)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->pitch(angle);
}

//----------------------------------------------------------------------------
void f3d_camera_set_current_as_default(f3d_camera_t* camera)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->setCurrentAsDefault();
}

//----------------------------------------------------------------------------
void f3d_camera_reset_to_default(f3d_camera_t* camera)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->resetToDefault();
}

//----------------------------------------------------------------------------
void f3d_camera_reset_to_bounds(f3d_camera_t* camera, double zoom_factor)
{
  if (!camera)
  {
    return;
  }

  f3d::camera* cpp_camera = reinterpret_cast<f3d::camera*>(camera);
  cpp_camera->resetToBounds(zoom_factor);
}
