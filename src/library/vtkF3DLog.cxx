#include "vtkF3DLog.h"

#include "vtkF3DConsoleOutputWindow.h"

#if F3D_WINDOWS_GUI
#include "vtkF3DWin32OutputWindow.h"
#endif

//----------------------------------------------------------------------------
void vtkF3DLog::Print(Severity sev, const std::string& str)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  switch (sev)
  {
    default:
    case vtkF3DLog::Severity::Info:
      win->DisplayText(str.c_str());
      break;
    case vtkF3DLog::Severity::Warning:
      win->DisplayWarningText(str.c_str());
      break;
    case vtkF3DLog::Severity::Error:
      win->DisplayErrorText(str.c_str());
      break;
  }
}

//----------------------------------------------------------------------------
void vtkF3DLog::SetUseColoring(bool use)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  vtkF3DConsoleOutputWindow* consoleWin = vtkF3DConsoleOutputWindow::SafeDownCast(win);
  if (consoleWin)
  {
    consoleWin->SetUseColoring(use);
  }
}

//----------------------------------------------------------------------------
void vtkF3DLog::SetQuiet(bool quiet)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  win->SetDisplayMode(quiet ? vtkOutputWindow::NEVER : vtkOutputWindow::ALWAYS);
}

//----------------------------------------------------------------------------
void vtkF3DLog::WaitForUser()
{
#if F3D_WINDOWS_GUI
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  vtkF3DWin32OutputWindow* win32Win = vtkF3DWin32OutputWindow::SafeDownCast(win);
  if (win32Win)
  {
    win32Win->WaitForUser();
  }
// No need for a #else as terminal output stays visible after
// F3D is closed.
#endif
}
