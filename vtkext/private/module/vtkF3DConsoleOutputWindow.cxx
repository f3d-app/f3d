#include "vtkF3DConsoleOutputWindow.h"

#include <vtkObjectFactory.h>

#include <iostream>

vtkStandardNewMacro(vtkF3DConsoleOutputWindow);

//----------------------------------------------------------------------------
vtkF3DConsoleOutputWindow::vtkF3DConsoleOutputWindow() = default;

//----------------------------------------------------------------------------
void vtkF3DConsoleOutputWindow::StartProgress()
{
  if (!this->ProgressStarted)
  {
    this->ProgressStarted = true;
    this->LastMessageIsProgress = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DConsoleOutputWindow::EndProgress()
{
  if (this->ProgressStarted)
  {
    this->ProgressStarted = false;

    // Shows empty text to add a newline after the progress bar
    if (this->LastMessageIsProgress)
    {
      this->DisplayText("");
    }

    this->LastMessageIsProgress = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DConsoleOutputWindow::DisplayText(const char* txt)
{
  std::string fmtText;
  if (this->ProgressStarted && this->LastMessageIsProgress)
  {
    fmtText += '\n';
  }

  if (this->UseColoring)
  {
    switch (this->GetCurrentMessageType())
    {
      case vtkOutputWindow::MESSAGE_TYPE_ERROR:
        fmtText += "\033[31;1m";
        fmtText += txt;
        fmtText += "\033[0m";
        break;
      case vtkOutputWindow::MESSAGE_TYPE_WARNING:
      case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
        fmtText += "\033[33m";
        fmtText += txt;
        fmtText += "\033[0m";
        break;
      default:
        fmtText += txt;
        break;
    }
  }
  else
  {
    fmtText += txt;
  }

  fmtText += '\n';

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

  this->LastMessageIsProgress = false;
}

//----------------------------------------------------------------------------
void vtkF3DConsoleOutputWindow::DisplayProgress(const char* str)
{
  if (!this->ProgressStarted)
  {
    return;
  }

  std::string fmtText = "\r";
  fmtText += str;

  this->Superclass::DisplayText(fmtText.c_str());

  switch (this->GetDisplayStream(this->GetCurrentMessageType()))
  {
    case vtkOutputWindow::StreamType::StdOutput:
      std::cout.flush();
      break;
    case vtkOutputWindow::StreamType::StdError:
      std::cerr.flush();
      break;
    // Other value that cannot be hit
    // LCOV_EXCL_START
    default:
      break;
      // LCOV_EXCL_STOP
  }

  this->LastMessageIsProgress = true;
}
