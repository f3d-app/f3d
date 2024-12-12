#include "F3DLog.h"

#include "vtkF3DConsoleOutputWindow.h"

F3DLog::Severity F3DLog::VerboseLevel = F3DLog::Severity::Info;

//----------------------------------------------------------------------------
void F3DLog::Print(Severity sev, const std::string& str)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  switch (sev)
  {
    default:
    case F3DLog::Severity::Debug:
      if (F3DLog::VerboseLevel <= F3DLog::Severity::Debug)
      {
        win->DisplayText(str.c_str());
      }
      break;
    case F3DLog::Severity::Info:
      if (F3DLog::VerboseLevel <= F3DLog::Severity::Info)
      {
        win->DisplayText(str.c_str());
      }
      break;
    case F3DLog::Severity::Warning:
      if (F3DLog::VerboseLevel <= F3DLog::Severity::Warning)
      {
        win->DisplayWarningText(str.c_str());
      }
      break;
    case F3DLog::Severity::Error:
      if (F3DLog::VerboseLevel <= F3DLog::Severity::Error)
      {
        win->DisplayErrorText(str.c_str());
      }
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
void F3DLog::SetStandardStream(StandardStream mode)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();

  switch (mode)
  {
    case StandardStream::None:
      win->SetDisplayMode(vtkOutputWindow::NEVER);
      break;
    case StandardStream::AlwaysStdErr:
      win->SetDisplayMode(vtkOutputWindow::ALWAYS_STDERR);
      break;
    case StandardStream::Default:
    default:
      win->SetDisplayMode(vtkOutputWindow::ALWAYS);
      break;
  }
}
