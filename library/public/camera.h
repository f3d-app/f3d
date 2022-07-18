/**
 * @class   camera
 * @brief   Abstract class to control a camera in a camera
 *
 * A class to control a camera in a window, use only after the first render of the window.
 */

#ifndef f3d_camera_h
#define f3d_camera_h

#include "export.h"

#include <array>
#include <stdexcept>

namespace f3d
{
class F3D_EXPORT camera
{
public:
  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  typedef std::array<double, 3> vector_t;
  typedef std::array<double, 16> matrix_t;

  virtual void setPosition(const vector_t& pos) = 0;
  virtual vector_t getPosition() = 0;
  virtual void setFocalPoint(const vector_t& foc) = 0;
  virtual vector_t getFocalPoint() = 0;
  virtual void setViewUp(const vector_t& up) = 0;
  virtual vector_t getViewUp() = 0;
  virtual void setViewAngle(const double& angle) = 0;
  virtual double getViewAngle() = 0;
  virtual void setViewMatrix(const matrix_t& matrix) = 0;
  virtual matrix_t getViewMatrix() = 0;

  virtual void dolly(double val) = 0;
  virtual void roll(double angle) = 0;
  virtual void azimuth(double angle) = 0;
  virtual void yaw(double angle) = 0;
  virtual void elevation(double angle) = 0;
  virtual void pitch(double angle) = 0;

  virtual void setCurrentAsDefault() = 0;
  virtual void resetToDefault() = 0;
  virtual void reset() = 0;

protected:
  camera() = default;
  virtual ~camera() = default;
  camera(const camera&) = delete;
  camera& operator=(const camera&) = delete;
};
}

#endif
