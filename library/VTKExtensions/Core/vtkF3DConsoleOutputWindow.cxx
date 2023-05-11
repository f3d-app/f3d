#include "vtkF3DConsoleOutputWindow.h"

#include <vtkObjectFactory.h>
#include <vtkWindows.h>

vtkStandardNewMacro(vtkF3DConsoleOutputWindow);

//----------------------------------------------------------------------------
#ifdef WIN32
vtkF3DConsoleOutputWindow::vtkF3DConsoleOutputWindow()
{
  // enable formatting on windows terminal
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode;
  GetConsoleMode(hOut, &dwMode);
  SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#else
vtkF3DConsoleOutputWindow::vtkF3DConsoleOutputWindow() = default;
#endif

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
