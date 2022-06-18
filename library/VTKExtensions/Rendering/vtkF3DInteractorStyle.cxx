#include "vtkF3DInteractorStyle.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
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

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  if (rwi->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }

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
  this->Superclass::Dolly();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Dolly(double factor)
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Dolly(factor);
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

    ren->GetSkybox()->SetFloorPlane(up[0], up[1], up[2], 0.0);
    ren->GetSkybox()->SetFloorRight(front[0], front[1], front[2]);

    this->Interactor->Render();
  }
}
