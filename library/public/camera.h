/**
 * @class   camera
 * @brief   Abstract class to control a camera in a camera
 *
 * A class to control a camera in a camera.
 */

#ifndef f3d_camera_h
#define f3d_camera_h

#include "export.h"

#include <array>

namespace f3d
{
class F3D_EXPORT camera
{
public:
  typedef std::array<double, 3> vector_t;
  typedef std::array<double, 16> matrix_t;

  virtual void setPosition(const vector_t& pos) = 0;
  virtual vector_t getPosition() = 0;
  virtual void setFocalPoint(const vector_t& foc) = 0;
  virtual vector_t getFocalPoint() = 0;
  virtual void setViewUp(const vector_t& up) = 0;
  virtual vector_t getViewUp() = 0;
  virtual void setViewMatrix(const matrix_t& matrix) = 0;
  virtual matrix_t getViewMatrix() = 0;

  virtual void dolly(double val) = 0;
  virtual void roll(double angle) = 0;
  virtual void azimuth(double angle) = 0;
  virtual void yaw(double angle) = 0;
  virtual void elevation(double angle) = 0;
  virtual void pitch(double angle) = 0;

protected:
  camera() = default;
  virtual ~camera() = default;
  camera(const camera&) = delete;
  camera& operator=(const camera&) = delete;
};
}

#endif
