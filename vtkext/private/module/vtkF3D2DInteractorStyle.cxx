#include "vtkF3D2DInteractorStyle.h"

#include "F3DLog.h"
#include "vtkF3DRenderer.h"

#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>

vtkStandardNewMacro(vtkF3D2DInteractorStyle);

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnLeftButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  this->StartPan();
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnLeftButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      break;
    default:
      break;
  }
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnMiddleButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  this->StartPan();
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnMiddleButtonUp()
{
  switch (this->State)
  {
    case VTKIS_PAN:
      this->EndPan();
      break;
    default:
      break;
  }
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnRightButtonDown()
{
  this->FindPokedRenderer(
    this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
  assert(this->CurrentRenderer != nullptr);

  this->StartDolly();
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnRightButtonUp()
{
  switch (this->State)
  {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;
    default:
      break;
  }
}

//----------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnDropFiles(vtkStringArray* files)
{
  if (files == nullptr)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Drop event without any provided files.");
    return;
  }
  this->InvokeEvent(vtkF3D2DInteractorStyle::DropFilesEvent, files);
}

//----------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::OnKeyPress()
{
  this->InvokeEvent(vtkF3D2DInteractorStyle::KeyPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::Pan()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::Dolly()
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
void vtkF3D2DInteractorStyle::Dolly(double factor)
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  if (this->Interactor->GetControlKey())
  {
    vtkF3D2DInteractorStyle::DollyToPosition(
      factor, this->Interactor->GetEventPosition(), this->CurrentRenderer);
  }
  else
  {
    this->Superclass::Dolly(factor);
  }
  this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::DollyToPosition(double factor, int* position, vtkRenderer* renderer)
{
  vtkCamera* cam = renderer->GetActiveCamera();
  double viewFocus[4], originalViewFocus[3], cameraPos[3], newCameraPos[3];
  double newFocalPoint[4], norm[3];

  cam->GetPosition(cameraPos);
  cam->GetFocalPoint(viewFocus);
  cam->GetFocalPoint(originalViewFocus);
  cam->GetViewPlaneNormal(norm);

  vtkF3D2DInteractorStyle::ComputeWorldToDisplay(
    renderer, viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);

  vtkF3D2DInteractorStyle::ComputeDisplayToWorld(
    renderer, double(position[0]), double(position[1]), viewFocus[2], newFocalPoint);

  cam->SetFocalPoint(newFocalPoint);
  cam->Dolly(factor);
  cam->GetPosition(newCameraPos);

  double newPoint[3];
  newPoint[0] = originalViewFocus[0] + newCameraPos[0] - cameraPos[0];
  newPoint[1] = originalViewFocus[1] + newCameraPos[1] - cameraPos[1];
  newPoint[2] = originalViewFocus[2] + newCameraPos[2] - cameraPos[2];

  cam->SetFocalPoint(newPoint);
}

//------------------------------------------------------------------------------
void vtkF3D2DInteractorStyle::UpdateRendererAfterInteraction()
{
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
void vtkF3D2DInteractorStyle::FindPokedRenderer(int vtkNotUsed(x), int vtkNotUsed(y))
{
  this->SetCurrentRenderer(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
}
