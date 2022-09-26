#include "vtkF3DInteractorStyle2D.h"

#include "F3DLog.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DInteractorStyle2D);

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::OnDropFiles(vtkStringArray* files)
{
  if (files == nullptr)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Drop event without any provided files.");
    return;
  }
  this->InvokeEvent(vtkF3DInteractorStyle2D::DropFilesEvent, files);
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::OnKeyPress()
{
  this->InvokeEvent(vtkF3DInteractorStyle2D::KeyPressEvent, nullptr);
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Rotate()
{
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Spin()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Spin();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Pan()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Dolly()
{
  if (this->CameraMovementDisabled)
  {
    return;
  }
  this->Superclass::Dolly();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle2D::Dolly(double factor)
{
  if (this->CameraMovementDisabled)
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
