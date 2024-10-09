#ifndef f3d_camera_h
#define f3d_camera_h

#include "export.h"
#include "types.h"

#include <array>
#include <string>

namespace f3d
{
struct F3D_EXPORT camera_state_t
{
  point3_t pos = { 0., 0., 1. };
  point3_t foc = { 0., 0., 0. };
  vector3_t up = { 0., 1., 0. };
  angle_deg_t angle = 30.;
};

/**
 * @class   camera
 * @brief   Abstract class to control a camera in a window
 *
 * A class to control a camera in a window.
 * When modifying the Position/FocalPoint/ViewUp API or the camera movements API,
 * the view up is automatically orthogonalized.
 * All angles are in degrees.
 */
class F3D_EXPORT camera
{
public:
  ///@{ @name Parameters
  /**
   * Set/Get the camera parameters.
   * Angles are in degrees.
   */
  virtual camera& setPosition(const point3_t& pos) = 0;
  virtual point3_t getPosition() = 0;
  virtual void getPosition(point3_t& pos) = 0;
  virtual camera& setFocalPoint(const point3_t& foc) = 0;
  virtual point3_t getFocalPoint() = 0;
  virtual void getFocalPoint(point3_t& foc) = 0;
  virtual camera& setViewUp(const vector3_t& up) = 0;
  virtual vector3_t getViewUp() = 0;
  virtual void getViewUp(vector3_t& up) = 0;
  virtual camera& setViewAngle(const angle_deg_t& angle) = 0;
  virtual angle_deg_t getViewAngle() = 0;
  virtual void getViewAngle(angle_deg_t& angle) = 0;
  virtual camera& setState(const camera_state_t& state) = 0;
  virtual camera_state_t getState() = 0;
  virtual void getState(camera_state_t& state) = 0;
  ///@}

  ///@{ @name Orientation
  /** Get the azimuth angle of the camera. */
  virtual angle_deg_t getAzimuth() = 0;
  /** Get the yaw angle of the camera. */
  virtual angle_deg_t getYaw() = 0;
  /** Get the elevation angle of the camera. */
  virtual angle_deg_t getElevation() = 0;
  ///@}

  ///@{ @name Manipulation
  /// Standard camera manipulation methods. Angles are in degrees.

  /** Divide the camera's distance from the focal point by the given value. */
  virtual camera& dolly(double val) = 0;
  /** Move the camera along its horizontal, vertical, and forward axes */
  virtual camera& pan(double right, double up, double forward = 0) = 0;
  /** Decrease the view angle (or the parallel scale in parallel mode) by the specified factor. */
  virtual camera& zoom(double factor) = 0;
  /** Rotate the camera about its forward axis. */
  virtual camera& roll(angle_deg_t angle) = 0;
  /** Rotate the camera about its vertical axis, centered at the focal point. */
  virtual camera& addAzimuth(angle_deg_t angle) = 0;
  /** Rotate the camera about its vertical axis, centered the camera's position. */
  virtual camera& addYaw(angle_deg_t angle) = 0;
  /** Rotate the camera about its horizontal axis, centered at the focal point. */
  virtual camera& addElevation(angle_deg_t angle) = 0;
  /** Rotate the camera about its horizontal axis, centered the camera's position. */
  virtual camera& pitch(angle_deg_t angle) = 0;

  ///@}

  /**
   * Store the current camera configuration as default.
   */
  virtual camera& setCurrentAsDefault() = 0;

  /**
   * Reset the camera to the stored default camera configuration.
   */
  virtual camera& resetToDefault() = 0;

  /**
   * Reset the camera using the bounds of actors in the scene.
   * Provided zoomFactor will be used to position the camera.
   * A value of 1 correspond to the bounds roughly aligned to the edges
   * of the window.
   */
  virtual camera& resetToBounds(double zoomFactor = 0.9) = 0;

protected:
  //! @cond
  camera() = default;
  virtual ~camera() = default;
  camera(const camera&) = delete;
  camera(camera&&) = delete;
  camera& operator=(const camera&) = delete;
  camera& operator=(camera&&) = delete;
  //! @endcond
};
}

#endif