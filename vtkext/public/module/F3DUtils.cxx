#include "F3DUtils.h"

#include <vtkObject.h>
#include <vtkSetGet.h>

#ifdef _MSC_VER
#include <Windows.h>
#elif VTK_USE_X
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
double F3DUtils::getDPIScale()
{
  double dpiScale = 1.0;
  constexpr int baseDPI = 96;

#ifdef _WIN32
  UINT dpi = GetDeviceCaps(wglGetCurrentDC(), LOGPIXELSY);

  if (dpi > 0)
  {
    dpiScale = static_cast<double>(dpi) / baseDPI;
  }
  else
  {
    vtkWarningWithObjectMacro(nullptr, "Fail to detect primary monitor DPI.");
  }
#elif VTK_USE_X
  Display* dpy = XOpenDisplay(nullptr);

  if (dpy)
  {
    unsigned int dpi = std::atoi(XGetDefault(dpy, "Xft", "dpi"));
    if (dpi > 0)
    {
      dpiScale = static_cast<double>(dpi) / baseDPI;
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

  XCloseDisplay(dpy);
#endif

  return dpiScale;
}
