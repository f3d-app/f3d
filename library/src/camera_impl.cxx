#include "camera_impl.h"

#include <vtkNew.h>
#include <vtkMatrix4x4.h>

#include <vtkExternalOpenGLCamera.h>

namespace f3d::detail
{
class camera_impl::internals
{
public:
  vtkNew<vtkExternalOpenGLCamera> VTKCamera;
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
  this->Internals->VTKCamera->SetViewTransformMatrix(matrix.data());
}

//----------------------------------------------------------------------------
std::array<double, 16> camera_impl::getViewMatrix()
{
  vtkMatrix4x4* mat = this->Internals->VTKCamera->GetModelViewTransformMatrix();
  double* data = mat->GetData();
  std::array<double, 16> arr;
  std::move(data, data + 16, arr.begin());
  return arr;
}

//----------------------------------------------------------------------------
void camera_impl::CopyFromVTKCamera(vtkCamera* cam)
{
  if (cam != this->Internals->VTKCamera)
  {
    this->Internals->VTKCamera->DeepCopy(cam);
  }
}

//----------------------------------------------------------------------------
vtkCamera* camera_impl::GetVTKCamera()
{
  return this->Internals->VTKCamera;
}
};
