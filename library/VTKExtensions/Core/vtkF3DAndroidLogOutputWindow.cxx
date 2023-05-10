#include "vtkF3DAndroidLogOutputWindow.h"

#include <vtkObjectFactory.h>

#include <android/log.h>

vtkStandardNewMacro(vtkF3DAndroidLogOutputWindow);

vtkF3DAndroidLogOutputWindow::vtkF3DAndroidLogOutputWindow() = default;

void vtkF3DAndroidLogOutputWindow::DisplayText(const char* txt)
{
  int prio = ANDROID_LOG_INFO;
  switch (this->GetCurrentMessageType())
  {
    case vtkOutputWindow::MESSAGE_TYPE_ERROR:
      prio = ANDROID_LOG_ERROR;
      break;
    case vtkOutputWindow::MESSAGE_TYPE_WARNING:
    case vtkOutputWindow::MESSAGE_TYPE_GENERIC_WARNING:
      prio = ANDROID_LOG_WARN;
      break;
    default:
      break;
  }

  __android_log_print(prio, "F3DLog", "%s", txt);
}
