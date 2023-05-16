#include "F3DLog.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DConsoleOutputWindow.h"

#if F3D_WINDOWS_GUI
#include "vtkF3DWin32OutputWindow.h"
#endif

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
void F3DLog::SetQuiet(bool quiet)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  win->SetDisplayMode(quiet ? vtkOutputWindow::NEVER : vtkOutputWindow::ALWAYS);
}

//----------------------------------------------------------------------------
void F3DLog::WaitForUser()
{
#if F3D_WINDOWS_GUI
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  vtkF3DWin32OutputWindow* win32Win = vtkF3DWin32OutputWindow::SafeDownCast(win);
  if (win32Win && win->GetDisplayMode() != vtkOutputWindow::NEVER)
  {
    win32Win->WaitForUser();
  }
// No need for a #else as terminal output stays visible after
// F3D is closed.
#endif
}
