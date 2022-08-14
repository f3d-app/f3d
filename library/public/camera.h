/**
 * @class   camera
 * @brief   Abstract class to control a camera in a window
 *
 * A class to control a camera in a window.
 * There is in essence three API.
 * The Position/FocalPoint/ViewUp API, the ViewMatrix API and the camera movements API.
 * They can be mixed by using ViewMatrix API with the other APIs but it can result in unexpected
 * results.
 * When using the Position/FocalPoint/ViewUp API or the camera movements API,
 * the view up is automatically orthogonalized.
 * All angles are in degrees.
 */

#ifndef f3d_camera_h
#define f3d_camera_h

#include "export.h"

#include <array>
#include <string>

namespace f3d
{
class F3D_EXPORT camera
{
public:
  //@{
  /**
   * Camera specific types.
   */
  struct point3_t : std::array<double, 3>
  {
  };
  struct vector3_t : std::array<double, 3>
  {
  };
  struct matrix4_t : std::array<double, 16>
  {
  };
  typedef double angle_deg_t;
  //@}

  //@{
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
  //@}

  //@{
  /**
   * Set/Get the camera view matrix. Please note that when using the view matrix API, using the
   * other camera manipulation API may results in unexepected results due to normalization.
   */
  virtual camera& setViewMatrix(const matrix4_t& matrix) = 0;
  virtual matrix4_t getViewMatrix() = 0;
  virtual void getViewMatrix(matrix4_t& matrix) = 0;
  //@}

  //@{
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
  //@}

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
  camera() = default;
  virtual ~camera() = default;
  camera(const camera&) = delete;
  camera(camera&&) = delete;
  camera& operator=(const camera&) = delete;
  camera& operator=(camera&&) = delete;
};
}

#endif
