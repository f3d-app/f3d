#include "vtkF3DInteractorStyle3D.h"

#include "F3DAnimationManager.h"
#include "F3DIncludes.h"
#include "F3DLoader.h"
#include "F3DLog.h"
#include "vtkF3DInteractionHandler.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


vtkStandardNewMacro(vtkF3DInteractorStyle3D);

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::OnDropFiles(vtkStringArray* files)
{
  this->InteractionHandler->OnDropFiles(this, files);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::OnKeyPress()
{
  this->InteractionHandler->HandleKeyPress(this);
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::Rotate()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);

  if (ren == nullptr)
  {
    return;
  }

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

  bool canElevate = ren->UsingTrackball() || std::abs(dot) < 0.99 || !std::signbit(dot * ryf);

  camera->Azimuth(rxf);

  if (canElevate)
  {
    camera->Elevation(ryf);
  }

  if (!ren->UsingTrackball())
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
void vtkF3DInteractorStyle3D::Spin()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Spin();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::Pan()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::Dolly()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::Dolly(double factor)
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly(factor);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle3D::EnvironmentRotate()
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

//----------------------------------------------------------------------------
bool vtkF3DInteractorStyle3D::IsUserInteractionBlocked()
{
  return this->AnimationManager->IsPlaying() && this->Options->CameraIndex >= 0;
}
