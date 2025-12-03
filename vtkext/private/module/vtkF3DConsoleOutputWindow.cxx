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
  switch (this->GetCurrentMessageType())
  {
    case vtkOutputWindow::MESSAGE_TYPE_ERROR:
      fmtText = this->UseColoring ? "\033[31;1m" : "";
      break;
    case vtkOutputWindow::MESSAGE_TYPE_WARNING:
    case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
      fmtText = this->UseColoring ? "\033[33m" : "";
      break;
    default:
      break;
  }
  fmtText += txt;

  fmtText += this->UseColoring ? "\033[0m\n" : "\n";

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
