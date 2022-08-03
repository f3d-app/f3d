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

class vtkRenderer;
class vtkCamera;
namespace f3d::detail
{
class camera_impl : public camera
{
public:
  /**
   * Default constructor
   */
  camera_impl() noexcept;

  /**
   * Default destructor
   */
  ~camera_impl() noexcept override;

  //@{
  /**
   * Documented public API
   */
  camera& setPosition(const vector3_t& pos) noexcept override;
  vector3_t getPosition() noexcept override;
  camera& getPosition(vector3_t& pos) noexcept override;
  camera& setFocalPoint(const vector3_t& foc) noexcept override;
  vector3_t getFocalPoint() noexcept override;
  camera& getFocalPoint(vector3_t& foc) noexcept override;
  camera& setViewUp(const vector3_t& up) noexcept override;
  vector3_t getViewUp() noexcept override;
  camera& getViewUp(vector3_t& up) noexcept override;
  camera& setViewAngle(const double& angle) noexcept override;
  double getViewAngle() noexcept override;
  camera& getViewAngle(double& angle) noexcept override;
  camera& setViewMatrix(const matrix4_t& matrix) noexcept override;
  matrix4_t getViewMatrix() noexcept override;
  camera& getViewMatrix(matrix4_t& matrix) noexcept override;

  camera& dolly(double val) noexcept override;
  camera& roll(double angle) noexcept override;
  camera& azimuth(double angle) noexcept override;
  camera& yaw(double angle) noexcept override;
  camera& elevation(double angle) noexcept override;
  camera& pitch(double angle) noexcept override;

  camera& setCurrentAsDefault() noexcept override;
  camera& resetToDefault() noexcept override;
  camera& resetToBounds() noexcept override;
  //@}

  /**
   * Implementation only api.
   * Set the pointer to the renderer to control the camera of.
   */
  void SetVTKRenderer(vtkRenderer* renderer);

  /**
   * Implementation only api.
   * Recover a pointer to the vtkCamera currently being controlled.
   * This should only be called after the renderer have been set and initialized
   * so that a camera is available.
   */
  vtkCamera* GetVTKCamera();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
