#include "f3d_log.h"

#include "vtkF3DConsoleOutputWindow.h"

#if F3D_WINDOWS_GUI
#include "vtkF3DWin32OutputWindow.h"
#endif

namespace f3d
{
//----------------------------------------------------------------------------
void log::printInternal(Severity sev, const std::string& str)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  switch (sev)
  {
    default:
    case log::Severity::Info:
      win->DisplayText(str.c_str());
      break;
    case log::Severity::Warning:
      win->DisplayWarningText(str.c_str());
      break;
    case log::Severity::Error:
      win->DisplayErrorText(str.c_str());
      break;
  }
}

//----------------------------------------------------------------------------
void log::setUseColoring(bool use)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  vtkF3DConsoleOutputWindow* consoleWin = vtkF3DConsoleOutputWindow::SafeDownCast(win);
  if (consoleWin)
  {
    consoleWin->SetUseColoring(use);
  }
}

//----------------------------------------------------------------------------
void log::setQuiet(bool quiet)
{
  vtkOutputWindow* win = vtkOutputWindow::GetInstance();
  win->SetDisplayMode(quiet ? vtkOutputWindow::NEVER : vtkOutputWindow::ALWAYS);
}

//----------------------------------------------------------------------------
void log::waitForUser()
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
}
