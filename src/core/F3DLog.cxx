#include "F3DLog.h"

#include "Config.h"
#include "vtkF3DConsoleOutputWindow.h"

//----------------------------------------------------------------------------
void F3DLog::PrintInternal(Severity sev, const std::string& str)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  switch (sev)
  {
    default:
    case F3DLog::Severity::Info:
      win->DisplayText(str.c_str());
      break;
    case F3DLog::Severity::Warning:
      win->DisplayWarningText(str.c_str());
      break;
    case F3DLog::Severity::Error:
      win->DisplayErrorText(str.c_str());
      break;
  }
}

//----------------------------------------------------------------------------
void F3DLog::SetUseColoring(bool use)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  vtkF3DConsoleOutputWindow* consoleWin = vtkF3DConsoleOutputWindow::SafeDownCast(win);
  if (consoleWin)
  {
    consoleWin->SetUseColoring(use);
  }
}

//----------------------------------------------------------------------------
void F3DLog::SetQuiet(bool quiet)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  win->SetDisplayMode(quiet ? vtkOutputWindow::NEVER : vtkOutputWindow::ALWAYS);
}
