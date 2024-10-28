#include "vtkF3DInteractorStyle.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSkybox.h>
#include <vtkStringArray.h>

vtkStandardNewMacro(vtkF3DInteractorStyle);

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
  double dir[3];
  camera->GetDirectionOfProjection(dir);
  double* up = ren->GetUpVector();

  double dot = vtkMath::Dot(dir, up);

  bool canElevate = ren->GetUseTrackball() || std::abs(dot) < 0.99 || !std::signbit(dot * ryf);

  camera->Azimuth(rxf);

  if (canElevate)
  {
    camera->Elevation(ryf);
  }

  if (!ren->GetUseTrackball())
  {
    // orthogonalize up vector based on focal direction
    vtkMath::MultiplyScalar(dir, dot);
    vtkMath::Subtract(up, dir, dir);
    vtkMath::Normalize(dir);
    camera->SetViewUp(dir);
  }
  else
  {
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
