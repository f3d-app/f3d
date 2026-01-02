#include "vtkF3DConsoleOutputWindow.h"

#include <vtkObjectFactory.h>

#include <iostream>

vtkStandardNewMacro(vtkF3DConsoleOutputWindow);

//----------------------------------------------------------------------------
vtkF3DConsoleOutputWindow::vtkF3DConsoleOutputWindow() = default;

//----------------------------------------------------------------------------
void vtkF3DConsoleOutputWindow::DisplayText(const char* txt)
{
  std::string fmtText;

  if (this->UseColoring)
  {
    switch (this->GetCurrentMessageType())
    {
      case vtkOutputWindow::MESSAGE_TYPE_ERROR:
        fmtText = "\033[31;1m";
        fmtText += txt;
        fmtText += "\033[0m";
        break;
      case vtkOutputWindow::MESSAGE_TYPE_WARNING:
      case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
        fmtText = "\033[33m";
        fmtText += txt;
        fmtText += "\033[0m";
        break;
      default:
        fmtText = txt;
        break;
    }
  }
  else
  {
    fmtText = txt;
  }

  fmtText += "\n";
  this->Superclass::DisplayText(fmtText.c_str());

  switch (this->GetDisplayStream(this->GetCurrentMessageType()))
  {
    case StreamType::StdOutput:
      std::cout.flush();
      break;
    case StreamType::StdError:
      std::cerr.flush();
      break;
    default:
      break;
  }
}
