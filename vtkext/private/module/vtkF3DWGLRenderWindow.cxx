#include <vtkObjectFactory.h>

#include "vtkF3DWGLRenderWindow.h"

#include <Windows.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

constexpr auto IMMERSIVE_DARK_MODE_SUPPORTED_SINCE = 19041;

namespace
{
/**
 * Helper function to detect if the
 * Windows Build Number is equal or greater to a number
 */
bool IsWindowsBuildNumberOrGreater(int buildNumber)
{
  std::string value;
  bool result = vtksys::SystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion;CurrentBuildNumber",
    value);

  if (result == true)
  {
    try
    {
      return std::stoi(value) >= buildNumber;
    }
    catch (const std::invalid_argument&)
    {
    }
  }

  return false;
}

/**
 * Helper function to fetch a DWORD from windows registry.
 *
 * @param hKey A handle to an open registry key
 * @param subKey The path of registry key relative to 'hKey'
 * @param value The name of the registry value
 * @param dWord Variable to store the result in
 */
bool ReadRegistryDWord(
  HKEY hKey, const std::wstring& subKey, const std::wstring& value, DWORD& dWord)
{
  DWORD dataSize = sizeof(DWORD);
  LONG result =
    RegGetValueW(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, &dWord, &dataSize);

  return result == ERROR_SUCCESS;
}

/**
 * Helper function to detect user theme
 */
bool IsWindowsInDarkMode()
{
  std::wstring subKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");

  DWORD value{};

  if (::ReadRegistryDWord(HKEY_CURRENT_USER, subKey, L"AppsUseLightTheme", value))
  {
    return value == 0;
  }

  if (::ReadRegistryDWord(HKEY_CURRENT_USER, subKey, L"SystemUsesLightTheme", value))
  {
    return value == 0;
  }

  return false;
}
}

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DWGLRenderWindow);

//------------------------------------------------------------------------------
vtkF3DWGLRenderWindow::vtkF3DWGLRenderWindow() = default;

//------------------------------------------------------------------------------
vtkF3DWGLRenderWindow::~vtkF3DWGLRenderWindow() = default;

//------------------------------------------------------------------------------
void vtkF3DWGLRenderWindow::WindowInitialize()
{
  this->Superclass::WindowInitialize();

  // set dark mode if necessary
  if (::IsWindowsBuildNumberOrGreater(IMMERSIVE_DARK_MODE_SUPPORTED_SINCE))
  {
    HWND hwnd = this->WindowId;
    BOOL useDarkMode = ::IsWindowsInDarkMode();
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
  }

  // set HDPI
#if WINVER >= 0x0605
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
}
