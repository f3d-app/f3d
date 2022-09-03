#include "camera_impl.h"

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>

namespace f3d::detail
{
class camera_impl::internals
{
public:
  vtkRenderer* VTKRenderer = nullptr;
  vtkNew<vtkCamera> DefaultVTKCamera;
};

//----------------------------------------------------------------------------
camera_impl::camera_impl()
  : Internals(std::make_unique<camera_impl::internals>())
{
}

//----------------------------------------------------------------------------
camera_impl::~camera_impl() = default;

//----------------------------------------------------------------------------
camera& camera_impl::setPosition(const point3_t& pos)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetPosition(pos.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
point3_t camera_impl::getPosition()
{
  point3_t pos;
  this->getPosition(pos);
  return pos;
}

//----------------------------------------------------------------------------
void camera_impl::getPosition(point3_t& pos)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetPosition(pos.data());
}

//----------------------------------------------------------------------------
camera& camera_impl::setFocalPoint(const point3_t& foc)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetFocalPoint(foc.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
point3_t camera_impl::getFocalPoint()
{
  point3_t foc;
  this->getFocalPoint(foc);
  return foc;
}

//----------------------------------------------------------------------------
void camera_impl::getFocalPoint(point3_t& foc)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetFocalPoint(foc.data());
}

//----------------------------------------------------------------------------
camera& camera_impl::setViewUp(const vector3_t& up)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetViewUp(up.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
vector3_t camera_impl::getViewUp()
{
  vector3_t up;
  this->getViewUp(up);
  return up;
}

//----------------------------------------------------------------------------
void camera_impl::getViewUp(vector3_t& up)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetViewUp(up.data());
}

//----------------------------------------------------------------------------
camera& camera_impl::setViewAngle(const angle_deg_t& angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetViewAngle(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
angle_deg_t camera_impl::getViewAngle()
{
  angle_deg_t angle;
  this->getViewAngle(angle);
  return angle;
}

//----------------------------------------------------------------------------
void camera_impl::getViewAngle(angle_deg_t& angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  angle = cam->GetViewAngle();
}

//----------------------------------------------------------------------------
camera& camera_impl::dolly(double val)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Dolly(val);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::roll(angle_deg_t angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Roll(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::azimuth(angle_deg_t angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Azimuth(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::yaw(angle_deg_t angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Yaw(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::elevation(angle_deg_t angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Elevation(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::pitch(angle_deg_t angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Pitch(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::setCurrentAsDefault()
{
  vtkCamera* cam = this->GetVTKCamera();
  this->Internals->DefaultVTKCamera->DeepCopy(cam);
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::resetToDefault()
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->DeepCopy(this->Internals->DefaultVTKCamera);
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::resetToBounds()
{
  this->Internals->VTKRenderer->ResetCamera();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
void camera_impl::SetVTKRenderer(vtkRenderer* renderer)
{
  this->Internals->VTKRenderer = renderer;
}

//----------------------------------------------------------------------------
vtkCamera* camera_impl::GetVTKCamera()
{
  return this->Internals->VTKRenderer->GetActiveCamera();
}
};
