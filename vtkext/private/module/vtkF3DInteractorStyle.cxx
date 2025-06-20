#include "vtkF3DInteractorStyle.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSkybox.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkF3DInteractorStyle);

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnLeftButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  if (this->Interactor->GetShiftKey())
  {
    this->StartPan();
  }
  else
  {
    if (this->Interactor->GetControlKey())
    {
      this->StartSpin();
    }
    else
    {
      this->StartRotate();
    }
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnLeftButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      break;
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnMiddleButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  this->StartPan();
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnMiddleButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      break;
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnRightButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  if (this->Interactor->GetShiftKey())
  {
    this->StartEnvRotate();
  }
  else
  {
    this->StartDolly();
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnRightButtonUp()
{
  switch (this->State)
  {
    case VTKIS_ENV_ROTATE:
      this->EndEnvRotate();
      break;

    case VTKIS_DOLLY:
      this->EndDolly();
      break;
  }
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnDropFiles(vtkStringArray* files)
{
  if (files == nullptr)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Drop event without any provided files.");
    return;
  }
  this->InvokeEvent(vtkF3DInteractorStyle::DropFilesEvent, files);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnKeyPress()
{
  this->InvokeEvent(vtkF3DInteractorStyle::KeyPressEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::Rotate()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);
  vtkRenderWindowInteractor* rwi = this->Interactor;

  int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

  const int* size = ren->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = dx * delta_azimuth * this->MotionFactor;
  double ryf = dy * delta_elevation * this->MotionFactor;

  vtkCamera* camera = ren->GetActiveCamera();

  if (!ren->GetUseTrackball())
  {
    double up[3];
    this->InterpolateTemporaryUp(0.1, ren->GetUpVector(), up);

    double envUpCamDirCross[3];
    vtkMath::Cross(up, camera->GetDirectionOfProjection(), envUpCamDirCross);
    constexpr double EPSILON = 128 * std::numeric_limits<double>::epsilon();
    if (vtkMath::Norm(envUpCamDirCross) < EPSILON)
    {
      // Keep setting the temporary up to the camera's up vector until the interpolated up vector
      // and the camera direction vector are not collinear
      this->SetTemporaryUp(camera->GetViewUp());
      this->InterpolateTemporaryUp(0.1, ren->GetUpVector(), up);
    }

    // Rotate camera around the focal point about the environment's up vector
    vtkNew<vtkTransform> Transform;
    Transform->Identity();
    const double* fp = camera->GetFocalPoint();
    Transform->Translate(+fp[0], +fp[1], +fp[2]);
    Transform->RotateWXYZ(rxf, ren->GetUpVector());
    Transform->Translate(-fp[0], -fp[1], -fp[2]);
    Transform->TransformPoint(camera->GetPosition(), camera->GetPosition());

    camera->SetViewUp(up);

    // Clamp parameter to `camera->Elevation()` to maintain -90 < elevation < +90
    constexpr double maxAbsElevation = 90 - 1e-10;
    const double elevation = vtkMath::DegreesFromRadians(
      vtkMath::AngleBetweenVectors(ren->GetUpVector(), camera->GetDirectionOfProjection()) -
      vtkMath::Pi() / 2);
    camera->Elevation(std::clamp(ryf, -maxAbsElevation - elevation, +maxAbsElevation - elevation));

    camera->OrthogonalizeViewUp();
  }
  else
  {
    camera->Azimuth(rxf);
    camera->Elevation(ryf);
    camera->OrthogonalizeViewUp();
  }

  this->UpdateRendererAfterInteraction();

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Spin()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Spin();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Pan()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Dolly()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }

  assert(this->CurrentRenderer != nullptr);

  vtkRenderWindowInteractor* rwi = this->Interactor;
  const double* center = this->CurrentRenderer->GetCenter();
  const int* current_position = rwi->GetEventPosition();
  const int* last_position = rwi->GetLastEventPosition();
  const int dy = current_position[1] - last_position[1];
  const int dx = current_position[0] - last_position[0];
  const double dxf = this->MotionFactor * dx / center[0];
  const double dyf = this->MotionFactor * dy / center[1];
  double dtf = std::abs(dyf) > std::abs(dxf) ? dyf : dxf;
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);
  if (ren && ren->GetInvertZoom())
  {
    dtf *= -1.;
  }
  this->Dolly(pow(1.1, dtf));
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Dolly(double factor)
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  if (this->Interactor->GetControlKey())
  {
    vtkF3DInteractorStyle::DollyToPosition(
      factor, this->Interactor->GetEventPosition(), this->CurrentRenderer);
  }
  else
  {
    this->Superclass::Dolly(factor);
  }
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::DollyToPosition(double factor, int* position, vtkRenderer* renderer)
{
  vtkCamera* cam = renderer->GetActiveCamera();
  double viewFocus[4], originalViewFocus[3], cameraPos[3], newCameraPos[3];
  double newFocalPoint[4], norm[3];

  // Move focal point to cursor position
  cam->GetPosition(cameraPos);
  cam->GetFocalPoint(viewFocus);
  cam->GetFocalPoint(originalViewFocus);
  cam->GetViewPlaneNormal(norm);

  vtkF3DInteractorStyle::ComputeWorldToDisplay(
    renderer, viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);

  vtkF3DInteractorStyle::ComputeDisplayToWorld(
    renderer, double(position[0]), double(position[1]), viewFocus[2], newFocalPoint);

  cam->SetFocalPoint(newFocalPoint);

  // Move camera in/out along projection direction
  cam->Dolly(factor);

  // Find new focal point
  cam->GetPosition(newCameraPos);

  double newPoint[3];
  newPoint[0] = originalViewFocus[0] + newCameraPos[0] - cameraPos[0];
  newPoint[1] = originalViewFocus[1] + newCameraPos[1] - cameraPos[1];
  newPoint[2] = originalViewFocus[2] + newCameraPos[2] - cameraPos[2];

  cam->SetFocalPoint(newPoint);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::EnvironmentRotate()
{
  this->Superclass::EnvironmentRotate();

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);
  if (ren)
  {
    // update skybox orientation
    double* up = ren->GetEnvironmentUp();
    double* right = ren->GetEnvironmentRight();

    double front[3];
    vtkMath::Cross(right, up, front);

    ren->GetSkyboxActor()->SetFloorPlane(up[0], up[1], up[2], 0.0);
    ren->GetSkyboxActor()->SetFloorRight(front[0], front[1], front[2]);

    this->Interactor->Render();
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::UpdateRendererAfterInteraction()
{
  // Make sure this->CurrentRenderer is set
  this->FindPokedRenderer(0, 0);

  if (this->CurrentRenderer)
  {
    if (this->AutoAdjustCameraClippingRange)
    {
      this->CurrentRenderer->ResetCameraClippingRange();
    }

    vtkRenderWindowInteractor* rwi = this->Interactor;
    if (rwi->GetLightFollowCamera())
    {
      this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::FindPokedRenderer(int vtkNotUsed(x), int vtkNotUsed(y))
{
  // No need for picking, F3D interaction are only with the first renderer
  this->SetCurrentRenderer(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::ResetTemporaryUp()
{
  // Make sure this->CurrentRenderer is set
  this->FindPokedRenderer(0, 0);

  if (this->CurrentRenderer)
  {
    vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);
    SetTemporaryUp(ren->GetUpVector());
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::SetTemporaryUp(const double* tempUp)
{
  for (int i = 0; i < 3; i++)
  {
    this->TemporaryUp[i] = tempUp[i];
  }
  this->TemporaryUpFactor = 1.0;
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::InterpolateTemporaryUp(
  const double factorDelta, const double* target, double* output)
{
  this->TemporaryUpFactor = std::max(this->TemporaryUpFactor - factorDelta, 0.0);
  if (this->TemporaryUpFactor >= 0)
  {
    const double factor = (1.0 - std::cos(vtkMath::Pi() * this->TemporaryUpFactor)) * 0.5;
    for (int i = 0; i < 3; i++)
    {
      output[i] = factor * this->TemporaryUp[i] + (1.0 - factor) * target[i];
    }
    vtkMath::Normalize(output);
  }
}
