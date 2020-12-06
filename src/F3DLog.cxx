#include "F3DLog.h"

#include "Config.h"

#include <vtkOutputWindow.h>

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
