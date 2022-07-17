/**
 * @class   camera_impl
 * @brief   A concrete implementation of camera
 *
 * A concrete implementation of camera that can provide a camera to control in a window
 * It also defined implementation only API used by the libf3d.
 * See camera.h for the class documentation
 */

#ifndef f3d_camera_impl_h
#define f3d_camera_impl_h

#include "camera.h"

#include <memory>

class vtkCamera;
namespace f3d::detail
{
class camera_impl : public camera
{
public:
  /**
   */
  camera_impl();

  /**
   * Default destructor
   */
  ~camera_impl() override;

  //@{
  /**
   * Documented public API
   */
  void setPosition(const vector_t& pos) override;
  vector_t getPosition() override;
  void setFocalPoint(const vector_t& foc) override;
  vector_t getFocalPoint() override;
  void setViewUp(const vector_t& up) override;
  vector_t getViewUp() override;
  void setViewMatrix(const matrix_t& matrix) override;
  matrix_t getViewMatrix() override;

  void dolly(double val) override;
  void roll(double angle) override;
  void azimuth(double angle) override;
  void yaw(double angle) override;
  void elevation(double angle) override;
  void pitch(double angle) override;
  //@}

  /**
   * Implementation only API.
   */
  void SetVTKCamera(vtkCamera* vtkCamera);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
