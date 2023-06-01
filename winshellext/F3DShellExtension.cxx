#include <shlobj.h>

#include <codecvt>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "F3DShellExtensionClassFactory.h"
#include "RegistryHelpers.h"

// Unique F3D Thumbnail Provider GUID
static const GUID CLSID_F3DThumbnailProvider = { 0x0f3d0f3d0, 0x7d37, 0x4ccc,
  { 0x89, 0x99, 0x4f, 0xdd, 0xde, 0xbc, 0x5b, 0xe6 } };

// Handle the the DLL's module
HINSTANCE g_hInst = NULL;
long g_cDllRef = 0;

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
template<typename F>
void RunOnJSONExtensions(fs::path modulePath, F callback)
{
  // JSON plugin files are located in ${install_path}/share/f3d/plugins
  for (const auto& entry :
    fs::directory_iterator(modulePath.parent_path().parent_path() / "share/f3d/plugins"))
  {
    if (entry.is_regular_file())
    {
      auto f = entry.path();

      if (f.extension() == ".json")
      {
        auto root = nlohmann::json::parse(std::ifstream(f));

        auto readers = root.find("readers");

        if (readers != root.end() && readers.value().is_array())
        {
          for (auto& r : readers.value())
          {
            auto excludeThumb = r.find("exclude_thumbnailer").value();

            if (excludeThumb.is_boolean() && excludeThumb.get<bool>() == false)
            {
              auto exts = r.find("extensions");

              if (exts != r.end() && exts.value().is_array())
              {
                for (auto& e : exts.value())
                {
                  if (e.is_string())
                  {
                    std::wstring ret = L".";

                    std::wstring_convert<std::codecvt_utf8<wchar_t> > toUnicode;
                    ret += toUnicode.from_bytes(e.get<std::string>());

                    callback(ret);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
// Standard DLL functions
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void*)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    g_hInst = hInstance;
    DisableThreadLibraryCalls(hInstance);
  }
  return TRUE;
}

//------------------------------------------------------------------------------
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
  HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

  if (IsEqualCLSID(CLSID_F3DThumbnailProvider, rclsid))
  {
    hr = E_OUTOFMEMORY;

    F3DShellExtensionClassFactory* pClassFactory = new F3DShellExtensionClassFactory();
    if (pClassFactory)
    {
      hr = pClassFactory->QueryInterface(riid, ppv);
      pClassFactory->Release();
    }
  }

  return hr;
}

//------------------------------------------------------------------------------
STDAPI DllCanUnloadNow()
{
  return (g_cDllRef == 0) ? S_OK : S_FALSE;
}

//------------------------------------------------------------------------------
// Register the COM server and the f3d thumbnail handler.
STDAPI DllRegisterServer()
{
  wchar_t szModuleName[MAX_PATH];
  if (GetModuleFileName(g_hInst, szModuleName, ARRAYSIZE(szModuleName)) == 0)
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  // Register the component.
  HRESULT hr = RegisterInprocServer(
    szModuleName, CLSID_F3DThumbnailProvider, L"F3DShellExtension.F3DThumbnailProvider Class");
  if (!SUCCEEDED(hr))
  {
    return hr;
  }
  // Register the thumbnail handler. The thumbnail handler is associated
  // with the f3d file class.
  RunOnJSONExtensions(szModuleName,
    [&](const std::wstring& ext)
    { hr |= RegisterShellExtThumbnailHandler(ext.c_str(), CLSID_F3DThumbnailProvider); });

  if (SUCCEEDED(hr))
  {
    // Invalidate the thumbnail cache.
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
  }

  return hr;
}

//------------------------------------------------------------------------------
// Unregister the COM server and the f3d thumbnail handler.
STDAPI DllUnregisterServer()
{
  wchar_t szModuleName[MAX_PATH];
  if (GetModuleFileName(g_hInst, szModuleName, ARRAYSIZE(szModuleName)) == 0)
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  // Unregister the component.
  HRESULT hr = UnregisterInprocServer(CLSID_F3DThumbnailProvider);
  if (SUCCEEDED(hr))
  {
    // Unregister the thumbnail handler.
    RunOnJSONExtensions(szModuleName,
      [&](const std::wstring& ext) { hr |= UnregisterShellExtThumbnailHandler(ext.c_str()); });
  }

  return hr;
}

//------------------------------------------------------------------------------
// Notify the Windows Shell that file associations have changed.
STDAPI DllNotifyShell()
{
  // Invalidate the thumbnail cache.
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

  return S_OK;
}
