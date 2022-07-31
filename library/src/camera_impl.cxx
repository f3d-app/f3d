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
  : Internals(new camera_impl::internals())
{
}

//----------------------------------------------------------------------------
camera_impl::~camera_impl() = default;

//----------------------------------------------------------------------------
void camera_impl::setPosition(const vector3_t& pos)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetPosition(pos.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
camera::vector3_t camera_impl::getPosition()
{
  vector3_t pos;
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetPosition(pos.data());
  return pos;
}

//----------------------------------------------------------------------------
void camera_impl::setFocalPoint(const vector3_t& foc)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetFocalPoint(foc.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
camera::vector3_t camera_impl::getFocalPoint()
{
  vector3_t foc;
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetFocalPoint(foc.data());
  return foc;
}

//----------------------------------------------------------------------------
void camera_impl::setViewUp(const vector3_t& up)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetViewUp(up.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
camera::vector3_t camera_impl::getViewUp()
{
  vector3_t up;
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetViewUp(up.data());
  return up;
}

//----------------------------------------------------------------------------
void camera_impl::setViewAngle(const double& angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetViewAngle(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
double camera_impl::getViewAngle()
{
  vtkCamera* cam = this->GetVTKCamera();
  return cam->GetViewAngle();
}

//----------------------------------------------------------------------------
void camera_impl::setViewMatrix(const matrix4_t& matrix)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetPosition(matrix[12], matrix[13], matrix[14]);
  cam->SetFocalPoint(matrix[8] + matrix[12], matrix[9] + matrix[13], matrix[10] + matrix[14]);
  cam->SetViewUp(matrix[4], matrix[5], matrix[6]);
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
camera::matrix4_t camera_impl::getViewMatrix()
{
  vtkCamera* cam = this->GetVTKCamera();
  matrix4_t arr;
  vtkMatrix4x4* mat = cam->GetModelViewTransformMatrix();
  vtkNew<vtkMatrix4x4> tMat;
  vtkMatrix4x4::Transpose(mat, tMat);
  double* data = tMat->GetData();
  std::move(data, data + 16, arr.begin());
  return arr;
}

//----------------------------------------------------------------------------
void camera_impl::dolly(double val)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Dolly(val);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::roll(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Roll(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::azimuth(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Azimuth(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::yaw(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Yaw(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::elevation(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Elevation(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::pitch(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Pitch(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::setCurrentAsDefault()
{
  vtkCamera* cam = this->GetVTKCamera();
  this->Internals->DefaultVTKCamera->DeepCopy(cam);
}

//----------------------------------------------------------------------------
void camera_impl::resetToDefault()
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->DeepCopy(this->Internals->DefaultVTKCamera);
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::resetToBounds()
{
  this->Internals->VTKRenderer->ResetCamera();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void camera_impl::SetVTKRenderer(vtkRenderer* renderer)
{
  this->Internals->VTKRenderer = renderer;
}

//----------------------------------------------------------------------------
vtkCamera* camera_impl::GetVTKCamera()
{
  if (!this->Internals->VTKRenderer)
  {
    throw camera::exception("No camera available");
  }
  vtkCamera* cam = this->Internals->VTKRenderer->GetActiveCamera();
  if (!cam)
  {
    throw camera::exception("No camera available");
  }
  return cam;
}
};
