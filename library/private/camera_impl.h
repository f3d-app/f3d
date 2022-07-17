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
   * Documented public API TODO
   */
  void setViewMatrix(const std::array<double, 16>& matrix) override;
  std::array<double, 16> getViewMatrix() override;
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
