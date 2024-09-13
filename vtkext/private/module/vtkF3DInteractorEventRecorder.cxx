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
        // No change in interactor, so nothing to do
        return;
    }

    // if we already have an interactor then stop observing it
    if (this->Interactor)
    {
        // Do a cleanup before disabling the old interactor
        this->Interactor->RemoveObserver(this->EventCallbackCommand);
        this->SetEnabled(0); // Now it can be disabled
    }

    // Activate the new interactor and update the state
    this->Interactor = interactor;

    if (this->Interactor)
    {
        // Add the observers again or set up a new interactor if it is necessary
        this->Interactor->AddObserver(vtkCommand::AnyEvent, this->EventCallbackCommand);
        this->SetEnabled(1); // Enable the new interactor
    }

    this->Modified(); // Notify that the state has changed
}

void vtkF3DInteractorEventRecorder::WriteEvent(const char* eventId, int* position, int modifier, 
                                               int keyCode, int repeatCount, char* keySym, 
                                               void* callData)
{
    // Log the details
    std::cout << "Event ID: " << (eventId ? eventId : "NULL") << std::endl;
    std::cout << "Position: (" << position[0] << ", " << position[1] << ")" << std::endl;
    std::cout << "Modifier: " << modifier << std::endl;
    std::cout << "Key Code: " << keyCode << std::endl;
    std::cout << "Repeat Count: " << repeatCount << std::endl;
    std::cout << "Key Symbol: " << (keySym ? keySym : "NULL") << std::endl;

    // Handle callData
    if (callData)
    {
        // Assume callData is a pointer to a string.
        // The data type can be adjusted in the future.
        char* callDataStr = static_cast<char*>(callData);
        std::cout << "Call Data: " << callDataStr << std::endl;
    }
    else
    {
        std::cout << "Call Data: NULL" << std::endl;
    }

    // Call the base class version to handle the standard behavior
    this->vtkInteractorEventRecorder::WriteEvent(eventId, position, modifier, keyCode, 
                                                 repeatCount, keySym);
}

void vtkF3DInteractorEventRecorder::Clear()
{
    if (this->OutputStream)
    {
        // Reset the output stream (clearing any recorded events)
        this->OutputStream->clear();
        std::cout << "Recorder cleared." << std::endl;
    }

    // Optionally reset the internal state, such as stopping recording
    this->State = vtkInteractorEventRecorder::Start;
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
          self->WriteEvent(vtkCommand::GetStringFromEventId(event), rwi->GetEventPosition(), mod,
            rwi->GetKeyCode(), rwi->GetRepeatCount(), rwi->GetKeySym(), callData);
        }
    }
    self->OutputStream->flush();
  }
}
