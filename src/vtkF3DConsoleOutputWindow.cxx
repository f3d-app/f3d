#include "vtkF3DConsoleOutputWindow.h"

#include <vtkObjectFactory.h>

#ifdef WIN32
#include "windows.h"
#endif

vtkStandardNewMacro(vtkF3DConsoleOutputWindow);

vtkF3DConsoleOutputWindow::vtkF3DConsoleOutputWindow()
{
#ifdef WIN32
  // enable formatting on windows terminal
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode;
  GetConsoleMode(hOut, &dwMode);
  SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void vtkF3DConsoleOutputWindow::DisplayText(const char* txt)
{
  std::string fmtText;
  switch(this->GetCurrentMessageType())
  {
    case vtkOutputWindow::MESSAGE_TYPE_ERROR:
      fmtText = "\033[31;1m";
      break;
    case vtkOutputWindow::MESSAGE_TYPE_WARNING:
    case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
      fmtText = "\033[33m";
      break;
    default:
      break;
  }
  fmtText += txt;

  fmtText += "\033[0m\n";

  this->Superclass::DisplayText(fmtText.c_str());

  switch (this->GetDisplayStream(this->GetCurrentMessageType()))
  {
    case StreamType::StdOutput:
      cout.flush();
      break;
    case StreamType::StdError:
      cerr.flush();
      break;
    default:
      break;
  }
}
