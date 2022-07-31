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
  camera_impl();

  /**
   * Default destructor
   */
  ~camera_impl() override;

  //@{
  /**
   * Documented public API
   */
  void setPosition(const vector3_t& pos) override;
  vector3_t getPosition() override;
  void setFocalPoint(const vector3_t& foc) override;
  vector3_t getFocalPoint() override;
  void setViewUp(const vector3_t& up) override;
  vector3_t getViewUp() override;
  void setViewAngle(const double& angle) override;
  double getViewAngle() override;
  void setViewMatrix(const matrix4_t& matrix) override;
  matrix4_t getViewMatrix() override;

  void dolly(double val) override;
  void roll(double angle) override;
  void azimuth(double angle) override;
  void yaw(double angle) override;
  void elevation(double angle) override;
  void pitch(double angle) override;
  
  void setCurrentAsDefault() override;
  void resetToDefault() override;
  void resetToBounds() override;
  //@}

  /**
   * Implementation only api.
   * Set the pointer to the renderer to control the camera of.
   */
  void SetVTKRenderer(vtkRenderer* renderer);

  /**
   * Implementation only api.
   * Recover a pointer to the vtkCamera currently being controlled
   */
  vtkCamera* GetVTKCamera();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
