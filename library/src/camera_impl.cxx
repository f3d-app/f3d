#include "camera_impl.h"

#include <vtkCamera.h>
#include <vtkWeakPointer.h>
#include <vtkMatrix4x4.h>

namespace f3d::detail
{
class camera_impl::internals
{
public:
  vtkWeakPointer<vtkCamera> VTKCamera;
};

//----------------------------------------------------------------------------
camera_impl::camera_impl()
 : Internals(new camera_impl::internals())
{
}

//----------------------------------------------------------------------------
camera_impl::~camera_impl()
{
}

//----------------------------------------------------------------------------
void camera_impl::setViewMatrix(const std::array<double, 16>& matrix)
{
  // TODO Which API to implement ?
  this->Internals->VTKCamera->SetPosition(0.776126,-0.438658,24.556);
  this->Internals->VTKCamera->SetFocalPoint(0.776126,-0.438658,0);
  this->Internals->VTKCamera->SetViewUp(0,1,0);
}

//----------------------------------------------------------------------------
std::array<double, 16> camera_impl::getViewMatrix()
{
  vtkMatrix4x4* mat = this->Internals->VTKCamera->GetModelViewTransformMatrix();
  mat->Transpose();
  double* data = mat->GetData();
  std::array<double, 16> arr;
  std::move(data, data + 16, arr.begin());
  return arr;
}

//----------------------------------------------------------------------------
void camera_impl::SetVTKCamera(vtkCamera* cam)
{
  this->Internals->VTKCamera = cam;
}
};
