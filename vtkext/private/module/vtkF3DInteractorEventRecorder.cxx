#include "vtkF3DInteractorEventRecorder.h"

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVersion.h>

vtkStandardNewMacro(vtkF3DInteractorEventRecorder);

//------------------------------------------------------------------------------
vtkF3DInteractorEventRecorder::vtkF3DInteractorEventRecorder()
{
  // Override the process event callback
  this->EventCallbackCommand->SetCallback(vtkF3DInteractorEventRecorder::ProcessEvents);
}

//------------------------------------------------------------------------------
void vtkF3DInteractorEventRecorder::SetInteractor(vtkRenderWindowInteractor* interactor)
{
  if (interactor == this->Interactor)
  {
    return;
  }

  // if we already have an Interactor then stop observing it
  if (this->Interactor)
  {
    this->SetEnabled(0); // disable the old interactor
  }

  this->Interactor = interactor;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkF3DInteractorEventRecorder::ProcessEvents(
  vtkObject* object, unsigned long event, void* clientData, void* callData)
{
  vtkF3DInteractorEventRecorder* self =
    reinterpret_cast<vtkF3DInteractorEventRecorder*>(clientData);
  vtkRenderWindowInteractor* rwi = vtkRenderWindowInteractor::SafeDownCast(object);

  // all events are processed
  if (self->State == vtkF3DInteractorEventRecorder::Recording)
  {
    switch (event)
    {
      case vtkCommand::ModifiedEvent: // don't want these
        break;

      case vtkCommand::ExitEvent: // disable the recorder then exit
        self->Off();
        rwi->ExitCallback();
        break;

      default:
      {
        int mod = 0;
        if (rwi->GetShiftKey())
        {
          mod |= ModifierKey::ShiftKey;
        }
        if (rwi->GetControlKey())
        {
          mod |= ModifierKey::ControlKey;
        }
        if (rwi->GetAltKey())
        {
          mod |= ModifierKey::AltKey;
        }
        self->WriteEvent(vtkCommand::GetStringFromEventId(event), rwi->GetEventPosition(), mod,
          rwi->GetKeyCode(), rwi->GetRepeatCount(), rwi->GetKeySym(), callData);

        self->OutputStream->flush();
      }
    }
  }
}
