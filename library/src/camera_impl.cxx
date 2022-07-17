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
  this->Internals->VTKCamera->SetPosition(matrix[12], matrix[13], matrix[14]);
  this->Internals->VTKCamera->SetFocalPoint(matrix[8] + matrix[12], matrix[9] + matrix[13], matrix[10] + matrix[14]);
  this->Internals->VTKCamera->SetViewUp(matrix[4], matrix[5], matrix[6]);
}

//----------------------------------------------------------------------------
std::array<double, 16> camera_impl::getViewMatrix()
{
  // Is this correct ?
  std::array<double, 16> arr;
  double pos[3];
  this->Internals->VTKCamera->GetPosition(pos);
  double foc[3];
  this->Internals->VTKCamera->GetFocalPoint(foc);
  double up[3];
  this->Internals->VTKCamera->GetViewUp(up);
  double to[3];
  vtkMath::Subtract(foc, pos, to);
  double right[3];
  vtkMath::Cross(to, up, right);

  std::move(std::begin(right), std::end(right), arr.begin());
  arr[3] = 0;
  std::move(std::begin(up), std::end(up), arr.begin() + 4);
  arr[7] = 0;
  std::move(std::begin(to), std::end(to), arr.begin() + 8);
  arr[11] = 0;
  std::move(std::begin(pos), std::end(pos), arr.begin() + 12);
  arr[15] = 1;
  return arr;
}

//----------------------------------------------------------------------------
void camera_impl::SetVTKCamera(vtkCamera* cam)
{
  this->Internals->VTKCamera = cam;
}
};
