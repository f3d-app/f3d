#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DConfigure.h"

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkStringArray.h>

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
  vtkObject* object, unsigned long event, void* clientData, void* vtkNotUsed(callData))
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
            rwi->GetKeyCode(), rwi->GetRepeatCount(), rwi->GetKeySym());
        }
    }
    self->OutputStream->flush();
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorEventRecorder::ReadEvent(std::string line)
{
  // Read events and invoke them on the object in question
  char event[256] = {}, keySym[256] = {};
  int pos[2], ctrlKey, shiftKey, altKey, keyCode, repeatCount;
  float tempf;

  std::istringstream iss(line);

  // Use classic locale, we don't want to parse float values with
  // user-defined locale.
  iss.imbue(std::locale::classic());

  iss.width(256);
  iss >> event;

  // Quick skip comment
  if (*event == '#')
  {
    // Parse the StreamVersion (not using >> since comment could be empty)
    // Expecting: # StreamVersion x.y

    if (strlen(line.c_str()) > 16 && !strncmp(line.c_str(), "# StreamVersion ", 16))
    {
      int res = sscanf(line.c_str() + 16, "%f", &tempf);
      if (res && res != EOF)
      {
        this->CurrentStreamVersion = tempf;
      }
    }
  }
  else
  {
    if (this->CurrentStreamVersion == 0)
    {
      vtkWarningMacro("StreamVersion has not been read, parsing may be incorrect");
    }

    unsigned long ievent = vtkCommand::GetEventIdFromString(event);
    if (ievent != vtkCommand::NoEvent)
    {
      iss >> pos[0];
      iss >> pos[1];
      if (this->CurrentStreamVersion >= 1.1)
      {
        int m;
        iss >> m;
        shiftKey = (m & ModifierKey::ShiftKey) ? 1 : 0;
        ctrlKey = (m & ModifierKey::ControlKey) ? 1 : 0;
        altKey = (m & ModifierKey::AltKey) ? 1 : 0;
      }
      else
      {
        iss >> ctrlKey;
        iss >> shiftKey;
        altKey = 0;
      }
      iss >> keyCode;
      iss >> repeatCount;
      iss >> keySym;

      vtkSmartPointer<vtkStringArray> strArr;
      vtkIdType nStr;
      iss >> nStr;
      if (nStr > 0)
      {
        strArr = vtkSmartPointer<vtkStringArray>::New();
        for (int i = 0; i < nStr; i++)
        {
          std::string str;
          iss >> str;
          strArr->InsertNextValue(str);
        }
      }

      this->Interactor->SetEventPosition(pos);
      this->Interactor->SetControlKey(ctrlKey);
      this->Interactor->SetShiftKey(shiftKey);
      this->Interactor->SetAltKey(altKey);
      this->Interactor->SetKeyCode(static_cast<char>(keyCode));
      this->Interactor->SetRepeatCount(repeatCount);
      this->Interactor->SetKeySym(keySym);

      this->Interactor->InvokeEvent(ievent, strArr);
    }
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorEventRecorder::WriteEvent(
  const char* event, int pos[2], int modifiers, int keyCode, int repeatCount, char* keySym)
{
  *this->OutputStream << event << " " << pos[0] << " " << pos[1] << " " << modifiers << " "
                      << keyCode << " " << repeatCount << " ";
  if (keySym)
  {
    *this->OutputStream << keySym << " ";
  }
  else
  {
    *this->OutputStream << "0 ";
  }

  // Add a trailing 0 for potential file paths to add.
  *this->OutputStream << "0\n";
}
