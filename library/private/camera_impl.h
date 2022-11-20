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

  ///@{
  /**
   * Documented public API
   */
  camera& setPosition(const point3_t& pos) override;
  point3_t getPosition() override;
  void getPosition(point3_t& pos) override;
  camera& setFocalPoint(const point3_t& foc) override;
  point3_t getFocalPoint() override;
  void getFocalPoint(point3_t& foc) override;
  camera& setViewUp(const vector3_t& up) override;
  vector3_t getViewUp() override;
  void getViewUp(vector3_t& up) override;
  camera& setViewAngle(const angle_deg_t& angle) override;
  angle_deg_t getViewAngle() override;
  void getViewAngle(angle_deg_t& angle) override;

  camera& dolly(double val) override;
  camera& roll(angle_deg_t angle) override;
  camera& azimuth(angle_deg_t angle) override;
  camera& yaw(angle_deg_t angle) override;
  camera& elevation(angle_deg_t angle) override;
  camera& pitch(angle_deg_t angle) override;

  camera& setCurrentAsDefault() override;
  camera& resetToDefault() override;
  camera& resetToBounds() override;
  ///@}

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
