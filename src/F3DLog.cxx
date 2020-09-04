#include "F3DLog.h"

#include "Config.h"

#if F3D_WIN32_APP
#include <Windows.h>
#endif

#include <iostream>

void F3DLog::PrintInternal(Severity sev, const std::string& str)
{
  std::ostream& outStream = (sev > F3DLog::Severity::Info) ? std::cerr : std::cout;
#if F3D_WIN32_APP
  char* buf = nullptr;
  size_t sz = 0;
  _dupenv_s(&buf, &sz, "F3D_NO_MESSAGEBOX");
  if (buf == nullptr)
  {
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

    MessageBox(0, str.c_str(), f3d::AppTitle.c_str(), icon);
  }
  else
  {
    outStream << str << std::endl;
  }
#else
  (void)sev;
  outStream << str << std::endl;
#endif
}
