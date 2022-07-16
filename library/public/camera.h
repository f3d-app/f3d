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
  virtual void setViewMatrix(const std::array<double, 16>& matrix) = 0;
  virtual std::array<double, 16> getViewMatrix() = 0;

protected:
  camera() = default;
  virtual ~camera() = default;
  camera(const camera&) = delete;
  camera& operator=(const camera&) = delete;
};
}

#endif
