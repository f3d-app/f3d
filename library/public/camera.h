#ifndef f3d_camera_h
#define f3d_camera_h

#include "export.h"
#include "types.h"

#include <array>
#include <string>

namespace f3d
{
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
  ///@}

  ///@{ @name Manipulation
  /**
   * Standard camera manipulation methods.
   * Angles are in degrees.
   */
  virtual camera& dolly(double val) = 0;
  virtual camera& roll(angle_deg_t angle) = 0;
  virtual camera& azimuth(angle_deg_t angle) = 0;
  virtual camera& yaw(angle_deg_t angle) = 0;
  virtual camera& elevation(angle_deg_t angle) = 0;
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
   */
  virtual camera& resetToBounds() = 0;

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
