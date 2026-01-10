#include "F3DUtils.h"

#include <vtkObject.h>
#include <vtkSetGet.h>
#include <vtkWindow.h>
#include <vtkWindows.h>

#ifdef _MSC_VER
#pragma comment(lib, "Shcore")
#include <ShellScalingApi.h>
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#include <X11/Xlib.h>
#endif

#include <charconv>
#include <stdexcept>

//----------------------------------------------------------------------------
double F3DUtils::ParseToDouble(const std::string& str, double def, const std::string& nameError)
{
  double value = def;
  if (!str.empty())
  {
    try
    {
      value = std::stod(str);
    }
    catch (const std::invalid_argument&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
    }
    catch (const std::out_of_range&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
    }
  }
  return value;
}

//----------------------------------------------------------------------------
int F3DUtils::ParseToInt(const std::string& str, int def, const std::string& nameError)
{
  int value = def;
  if (!str.empty())
  {
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);

    if (result.ec == std::errc::result_out_of_range)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
      return def;
    }
    else if (result.ec != std::errc() || *(result.ptr) != '\0')
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
      return def;
    }
  }
  return value;
}

//----------------------------------------------------------------------------
double F3DUtils::getDPIScale(vtkWindow* win)
{
  double dpiScale = 1.0;

#ifdef _WIN32
  if (!win)
  // Get primary monitor dpi for app start up
  {
    // A legacy API that does not reflect DPI change at runtime.
    // Minimum supported client: Windows 2000 Professional [desktop apps only]
    UINT dpi = GetDeviceCaps(wglGetCurrentDC(), LOGPIXELSY);

    if (dpi > 0)
    {
      dpiScale = static_cast<double>(dpi) / 96;
    }
    else
    {
      vtkWarningWithObjectMacro(nullptr, "Fail to detect primary monitor DPI.");
    }
  }
  else
  // Get monitor dpi where the app window currently locate.
  // Handle window dragging between multi-monitors configuration
  // and system scale change at run-time.
  {
    HWND winID = static_cast<HWND>(win->GetGenericWindowId());

    if (winID)
    {
      HMONITOR hmon = MonitorFromWindow(winID, MONITOR_DEFAULTTONEAREST);

      if (hmon)
      {
        UINT dpiX = 0, dpiY = 0;
        // A more modern API that able to detect DPI dynamically.
        // Minimum supported client: Windows 8.1 [desktop apps only]
        HRESULT hres = GetDpiForMonitor(hmon, MDT_DEFAULT, &dpiX, &dpiY);

        if (hres == S_OK && dpiX > 0)
        {
          dpiScale = static_cast<double>(dpiX) / 96;
        }
        else
        {
          vtkWarningWithObjectMacro(nullptr, "Fail to detect current window DPI.");
        }
      }
      else
      {
        vtkWarningWithObjectMacro(nullptr, "Fail to Get HMONITOR.");
      }
    }
    else
    {
      vtkWarningWithObjectMacro(nullptr, "Fail to get window ID.");
    }
  }
#elif defined(__linux__) && !defined(__ANDROID__)
  Display* dpy = nullptr;

  if (win)
  {
    dpy = static_cast<Display*>(win->GetGenericDisplayId());
  }
  else
  {
    dpy = XOpenDisplay(nullptr);
  }

  if (dpy)
  {
    unsigned int dpi = std::atoi(XGetDefault(dpy, "Xft", "dpi"));
    if (dpi > 0)
    {
      dpiScale = dpi / 96.0;
    }
    else
    {
      vtkWarningWithObjectMacro(nullptr, "Fail to get DPI.");
    }
  }
  else
  {
    vtkWarningWithObjectMacro(nullptr, "Fail to get Display.");
  }

  if (!win)
  {
    XCloseDisplay(dpy);
  }
#endif

  return dpiScale;
}
