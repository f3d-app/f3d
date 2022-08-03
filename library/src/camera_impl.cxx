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
camera& camera_impl::setPosition(const vector3_t& pos)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetPosition(pos.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera::vector3_t camera_impl::getPosition()
{
  vector3_t pos;
  this->getPosition(pos);
  return pos;
}

//----------------------------------------------------------------------------
camera& camera_impl::getPosition(vector3_t& pos)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetPosition(pos.data());
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::setFocalPoint(const vector3_t& foc)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetFocalPoint(foc.data());
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera::vector3_t camera_impl::getFocalPoint()
{
  vector3_t foc;
  this->getFocalPoint(foc);
  return foc;
}

//----------------------------------------------------------------------------
camera& camera_impl::getFocalPoint(vector3_t& foc)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetFocalPoint(foc.data());
  return *this;
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
camera::vector3_t camera_impl::getViewUp()
{
  vector3_t up;
  this->getViewUp(up);
  return up;
}

//----------------------------------------------------------------------------
camera& camera_impl::getViewUp(vector3_t& up)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->GetViewUp(up.data());
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::setViewAngle(const double& angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetViewAngle(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
double camera_impl::getViewAngle()
{
  double angle;
  this->getViewAngle(angle);
  return angle;
}

//----------------------------------------------------------------------------
camera& camera_impl::getViewAngle(double& angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  angle = cam->GetViewAngle();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::setViewMatrix(const matrix4_t& matrix)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->SetPosition(matrix[12], matrix[13], matrix[14]);
  cam->SetFocalPoint(matrix[8] + matrix[12], matrix[9] + matrix[13], matrix[10] + matrix[14]);
  cam->SetViewUp(matrix[4], matrix[5], matrix[6]);
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera::matrix4_t camera_impl::getViewMatrix()
{
  matrix4_t matrix;
  this->getViewMatrix(matrix);
  return matrix;
}

//----------------------------------------------------------------------------
camera& camera_impl::getViewMatrix(matrix4_t& matrix)
{
  vtkCamera* cam = this->GetVTKCamera();
  vtkMatrix4x4* mat = cam->GetModelViewTransformMatrix();
  vtkNew<vtkMatrix4x4> tMat;
  vtkMatrix4x4::Transpose(mat, tMat);
  double* data = tMat->GetData();
  std::move(data, data + 16, matrix.begin());
  return *this;
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
camera& camera_impl::roll(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Roll(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::azimuth(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Azimuth(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::yaw(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Yaw(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::elevation(double angle)
{
  vtkCamera* cam = this->GetVTKCamera();
  cam->Elevation(angle);
  cam->OrthogonalizeViewUp();
  this->Internals->VTKRenderer->ResetCameraClippingRange();
  return *this;
}

//----------------------------------------------------------------------------
camera& camera_impl::pitch(double angle)
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
