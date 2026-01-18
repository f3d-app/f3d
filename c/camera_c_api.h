#ifndef F3D_CAMERA_C_API_H
#define F3D_CAMERA_C_API_H

#include "export.h"
#include "types_c_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::camera object.
   */
  typedef struct f3d_camera_t f3d_camera_t;

  /**
   * @brief Structure containing all information to configure a camera.
   */
  typedef struct f3d_camera_state_t
  {
    f3d_point3_t position;
    f3d_point3_t focal_point;
    f3d_vector3_t view_up;
    f3d_angle_deg_t view_angle;
  } f3d_camera_state_t;

  /**
   * @brief Set the position of the camera.
   *
   * @param camera Camera handle.
   * @param pos Position array [x, y, z].
   */
  F3D_EXPORT void f3d_camera_set_position(f3d_camera_t* camera, const f3d_point3_t pos);

  /**
   * @brief Get the position of the camera.
   *
   * @param camera Camera handle.
   * @param pos Output position array [x, y, z].
   */
  F3D_EXPORT void f3d_camera_get_position(const f3d_camera_t* camera, f3d_point3_t pos);

  /**
   * @brief Set the focal point of the camera.
   *
   * @param camera Camera handle.
   * @param focal_point Focal point array [x, y, z].
   */
  F3D_EXPORT void f3d_camera_set_focal_point(f3d_camera_t* camera, const f3d_point3_t focal_point);

  /**
   * @brief Get the focal point of the camera.
   *
   * @param camera Camera handle.
   * @param focal_point Output focal point array [x, y, z].
   */
  F3D_EXPORT void f3d_camera_get_focal_point(const f3d_camera_t* camera, f3d_point3_t focal_point);

  /**
   * @brief Set the view up vector of the camera.
   *
   * @param camera Camera handle.
   * @param view_up View up vector [x, y, z].
   */
  F3D_EXPORT void f3d_camera_set_view_up(f3d_camera_t* camera, const f3d_vector3_t view_up);

  /**
   * @brief Get the camera world azimuth angle in degrees.
   *
   * @param camera Camera handle.
   * @return World elevation angle in degrees.
   */
  F3D_EXPORT f3d_angle_deg_t f3d_camera_get_world_azimuth(const f3d_camera_t* camera);

  /**
   * @brief Get the camera world elevation angle in degrees.
   *
   * @param camera Camera handle.
   * @return World elevation angle in degrees.
   */
  F3D_EXPORT f3d_angle_deg_t f3d_camera_get_world_elevation(const f3d_camera_t* camera);

  /**
   * @brief Get the distance between the camera position and its focal point.
   *
   * @param camera Camera handle.
   * @return Distance to focal point.
   */
  F3D_EXPORT double f3d_camera_get_distance(const f3d_camera_t* camera);

  /**
   * @brief Get the view up vector of the camera.
   *
   * @param camera Camera handle.
   * @param view_up Output view up vector [x, y, z].
   */
  F3D_EXPORT void f3d_camera_get_view_up(const f3d_camera_t* camera, f3d_vector3_t view_up);

  /**
   * @brief Set the view angle in degrees of the camera.
   *
   * @param camera Camera handle.
   * @param angle View angle in degrees.
   */
  F3D_EXPORT void f3d_camera_set_view_angle(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Get the view angle in degrees of the camera.
   *
   * @param camera Camera handle.
   * @return View angle in degrees.
   */
  F3D_EXPORT f3d_angle_deg_t f3d_camera_get_view_angle(const f3d_camera_t* camera);

  /**
   * @brief Set the complete state of the camera.
   *
   * @param camera Camera handle.
   * @param state Camera state structure.
   */
  F3D_EXPORT void f3d_camera_set_state(f3d_camera_t* camera, const f3d_camera_state_t* state);

  /**
   * @brief Get the complete state of the camera.
   *
   * @param camera Camera handle.
   * @param state Output camera state structure.
   */
  F3D_EXPORT void f3d_camera_get_state(const f3d_camera_t* camera, f3d_camera_state_t* state);

  /**
   * @brief Divide the camera's distance from the focal point by the given value.
   *
   * @param camera Camera handle.
   * @param val Value to divide distance by.
   */
  F3D_EXPORT void f3d_camera_dolly(f3d_camera_t* camera, double val);

  /**
   * @brief Move the camera along its horizontal, vertical, and forward axes.
   *
   * @param camera Camera handle.
   * @param right Movement along the right axis.
   * @param up Movement along the up axis.
   * @param forward Movement along the forward axis.
   */
  F3D_EXPORT void f3d_camera_pan(f3d_camera_t* camera, double right, double up, double forward);

  /**
   * @brief Decrease the view angle (or the parallel scale in parallel mode) by the specified
   * factor.
   *
   * @param camera Camera handle.
   * @param factor Zoom factor.
   */
  F3D_EXPORT void f3d_camera_zoom(f3d_camera_t* camera, double factor);

  /**
   * @brief Rotate the camera about its forward axis.
   *
   * @param camera Camera handle.
   * @param angle Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_camera_roll(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Rotate the camera about its vertical axis, centered at the focal point.
   *
   * @param camera Camera handle.
   * @param angle Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_camera_azimuth(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Rotate the camera about its vertical axis, centered at the camera's position.
   *
   * @param camera Camera handle.
   * @param angle Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_camera_yaw(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Rotate the camera about its horizontal axis, centered at the focal point.
   *
   * @param camera Camera handle.
   * @param angle Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_camera_elevation(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Rotate the camera about its horizontal axis, centered at the camera's position.
   *
   * @param camera Camera handle.
   * @param angle Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_camera_pitch(f3d_camera_t* camera, f3d_angle_deg_t angle);

  /**
   * @brief Store the current camera configuration as default.
   *
   * @param camera Camera handle.
   */
  F3D_EXPORT void f3d_camera_set_current_as_default(f3d_camera_t* camera);

  /**
   * @brief Reset the camera to the stored default camera configuration.
   *
   * @param camera Camera handle.
   */
  F3D_EXPORT void f3d_camera_reset_to_default(f3d_camera_t* camera);

  /**
   * @brief Reset the camera using the bounds of actors in the scene.
   *
   * Provided zoom_factor will be used to position the camera.
   * A value of 1 corresponds to the bounds roughly aligned to the edges of the window.
   *
   * @param camera Camera handle.
   * @param zoom_factor Zoom factor (default: 0.9).
   */
  F3D_EXPORT void f3d_camera_reset_to_bounds(f3d_camera_t* camera, double zoom_factor);

#ifdef __cplusplus
}
#endif

#endif // F3D_CAMERA_C_API_H
