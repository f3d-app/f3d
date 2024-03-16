#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DConfigure.h"

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

      default:
        if (rwi->GetKeySym() && rwi->GetKeySym() == std::string(F3D_EXIT_HOTKEY_SYM))
        {
          self->Off();
        }
        else
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
// Complete WriteEvent needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9199
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20220519)
          self->WriteEvent(vtkCommand::GetStringFromEventId(event), rwi->GetEventPosition(), mod,
            rwi->GetKeyCode(), rwi->GetRepeatCount(), rwi->GetKeySym(), callData);
#else
          (void)callData;
          self->WriteEvent(vtkCommand::GetStringFromEventId(event), rwi->GetEventPosition(), mod,
            rwi->GetKeyCode(), rwi->GetRepeatCount(), rwi->GetKeySym());
#endif
        }
    }
    self->OutputStream->flush();
  }
}
