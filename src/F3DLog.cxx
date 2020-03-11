#include "F3DLog.h"

#if F3D_WIN32_APP
#include <Windows.h>
#endif

#include <iostream>

void F3DLog::PrintInternal(Severity sev, const std::string& str)
{
#if F3D_WIN32_APP
  unsigned int icon;
  switch (sev)
  {
    default:
    case F3DLog::Severity::Info:
      icon = MB_ICONINFORMATION;
      break;
    case F3DLog::Severity::Warning:
      icon = MB_ICONWARNING;
      break;
    case F3DLog::Severity::Error:
      icon = MB_ICONERROR;
      break;
  }
  if (!std::getenv("F3D_NO_MESSAGEBOX"))
  {
    MessageBox(0, str.c_str(), f3d::AppTitle.c_str(), icon);
  }
#else
  (void)sev;
  std::cerr << str << std::endl;
#endif
}
