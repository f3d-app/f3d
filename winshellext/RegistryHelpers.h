#pragma once

#include <cstdio>
#include <windows.h>

#define RETURNONFAILURE(_hr)                                                                       \
  if (!SUCCEEDED(_hr))                                                                             \
  {                                                                                                \
    return hr;                                                                                     \
  }

//------------------------------------------------------------------------------
// This function creates and set a HKCU registry key string value
HRESULT RegSetHKCUKeyValue(const wchar_t* subKey, const wchar_t* valueName, const wchar_t* value)
{
  HKEY key = nullptr;
  HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CURRENT_USER, subKey, 0, nullptr,
    REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr));

  RETURNONFAILURE(hr);

  if (value != nullptr)
  {
    hr = HRESULT_FROM_WIN32(RegSetValueEx(key, valueName, 0, REG_SZ,
      reinterpret_cast<const BYTE*>(value), lstrlen(value) * sizeof(*value)));
  }
  RegCloseKey(key);
  return hr;
}

//------------------------------------------------------------------------------
// This function creates and set a HKCU registry key dword value
HRESULT RegSetHKCUKeyValue(const wchar_t* subKey, const wchar_t* valueName, DWORD value)
{
  HKEY hKey = nullptr;
  HRESULT hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CURRENT_USER, subKey, 0, nullptr,
    REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr));
  RETURNONFAILURE(hr);

  hr = HRESULT_FROM_WIN32(RegSetValueEx(
    hKey, valueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)));
  RegCloseKey(hKey);
  return hr;
}

//------------------------------------------------------------------------------
// Get the data from a HKCU registry subkey
HRESULT RegGetHKCUKeyValue(
  const wchar_t* subKey, const wchar_t* valueName, wchar_t* buffer, DWORD bufferLen)
{
  HKEY hKey = nullptr;
  HRESULT hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey));
  RETURNONFAILURE(hr);

  // Get the data for the specified value name.
  hr = HRESULT_FROM_WIN32(RegQueryValueEx(
    hKey, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferLen));
  RegCloseKey(hKey);
  return hr;
}

//------------------------------------------------------------------------------
// Register the in-process component in the registry.
HRESULT RegisterInprocServer(const wchar_t* module, const CLSID& clsid, const wchar_t* friendlyName)
{
  wchar_t clsidStr[MAX_PATH];
  StringFromGUID2(clsid, clsidStr, ARRAYSIZE(clsidStr));

  // Create the HKCU\CLSID\{<clsid>} key.
  wchar_t subkey[MAX_PATH];
  swprintf_s(subkey, MAX_PATH, L"Software\\Classes\\CLSID\\%s", clsidStr);
  HRESULT hr = RegSetHKCUKeyValue(subkey, nullptr, friendlyName);
  RETURNONFAILURE(hr);

  // Set the DisableProcessIsolation of the component.
  // This is important to enable thumbnail generation from file path.
  hr = RegSetHKCUKeyValue(subkey, L"DisableProcessIsolation", 1);
  RETURNONFAILURE(hr);

  // Create the HKCU\CLSID\{<clsid>}\InprocServer32 key.
  // and set the default value of the InprocServer32 key to the path of the module.
  swprintf_s(subkey, MAX_PATH, L"Software\\Classes\\CLSID\\%s\\InprocServer32", clsidStr);
  hr = RegSetHKCUKeyValue(subkey, nullptr, module);
  RETURNONFAILURE(hr);

  // Set the threading model of the component.
  hr = RegSetHKCUKeyValue(subkey, L"ThreadingModel", L"Apartment");
  return hr;
}

//------------------------------------------------------------------------------
// Unregister an in-process component in the registry
// by deleting the HKCU\CLSID\{<clsid>} key.
HRESULT UnregisterInprocServer(const CLSID& clsid)
{
  wchar_t clsidStr[MAX_PATH];
  StringFromGUID2(clsid, clsidStr, MAX_PATH);
  wchar_t subkey[MAX_PATH];
  swprintf_s(subkey, MAX_PATH, L"Software\\Classes\\CLSID\\%s", clsidStr);
  return HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CURRENT_USER, subkey));
}

//------------------------------------------------------------------------------
// Register a thumbnail handler.
HRESULT RegisterShellExtThumbnailHandler(const wchar_t* fileType, const CLSID& clsid)
{
  // If fileType starts with '.', try to read the default value of the
  // HKCU\Software\Classes\<fileType> key which contains the ProgID to which the file type is
  // linked.
  wchar_t subkey[MAX_PATH];
  wchar_t defaultVal[MAX_PATH];
  if (*fileType == L'.')
  {
    swprintf_s(subkey, MAX_PATH, L"Software\\Classes\\CLSID\\%s", fileType);
    HRESULT hr = RegGetHKCUKeyValue(subkey, nullptr, defaultVal, sizeof(defaultVal));

    // If the key exists and its default value is not empty, use the ProgID as the file type.
    if (SUCCEEDED(hr) && defaultVal[0] != L'\0')
    {
      fileType = defaultVal;
    }
  }

  // Create the registry key used for thumbnails providers
  // HKCU\<fileType>\shellex\{E357FCCD-A995-4576-B01F-234630154E96}
  wchar_t clsidStr[MAX_PATH];
  StringFromGUID2(clsid, clsidStr, ARRAYSIZE(clsidStr));
  swprintf_s(subkey, MAX_PATH,
    L"Software\\Classes\\%s\\shellex\\{E357FCCD-A995-4576-B01F-234630154E96}", fileType);
  // Set the default value of the key
  return RegSetHKCUKeyValue(subkey, nullptr, clsidStr);
}

//------------------------------------------------------------------------------
// Unregister a thumbnail handler.
HRESULT UnregisterShellExtThumbnailHandler(const wchar_t* fileType)
{
  // If fileType starts with '.', try to read the default value of the
  // HKCU\<fileType> key which contains the ProgID to which the file type is linked.
  wchar_t defaultVal[MAX_PATH];
  if (*fileType == L'.')
  {
    HRESULT hr = RegGetHKCUKeyValue(fileType, nullptr, defaultVal, sizeof(defaultVal));

    // If the key exists and its default value is not empty, use the ProgID as the file type.
    if (SUCCEEDED(hr) && defaultVal[0] != L'\0')
    {
      fileType = defaultVal;
    }
  }

  // Remove the registry key HKCU\<fileType>\shellex\{E357FCCD-A995-4576-B01F-234630154E96}
  wchar_t subkey[MAX_PATH];
  HRESULT hr = swprintf_s(subkey, MAX_PATH,
    L"Software\\Classes\\%s\\shellex\\{E357FCCD-A995-4576-B01F-234630154E96}", fileType);
  RETURNONFAILURE(hr);
  return HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CURRENT_USER, subkey));
}
