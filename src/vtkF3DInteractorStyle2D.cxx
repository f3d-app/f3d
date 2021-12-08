#include "vtkF3DInteractorStyle2D.h"

#include "F3DAnimationManager.h"
#include "F3DIncludes.h"
#include "F3DLoader.h"
#include "F3DLog.h"
#include "vtkF3DInteractionHandler.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DInteractorStyle2D);

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::OnDropFiles(vtkStringArray* files)
{
  vtkF3DInteractionHandler::GetInstance()->OnDropFiles(this, files);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::OnKeyPress()
{
  vtkF3DInteractionHandler::GetInstance()->HandleKeyPress(this);
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Rotate()
{
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Spin()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Spin();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Pan()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Dolly()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Dolly(double factor)
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly(factor);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::EnvironmentRotate()
{
  this->Superclass::EnvironmentRotate();
}

//----------------------------------------------------------------------------
bool vtkF3DInteractorStyle2D::IsUserInteractionBlocked()
{
  return this->AnimationManager->IsPlaying() && this->Options->CameraIndex >= 0;
}
