#include "F3DConfigFileTools.h"

#include "F3DConfig.h"

#include "log.h"

#include <cstring>
#include <filesystem>
#include <vector>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <libloaderapi.h>
// clang-format on
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

//----------------------------------------------------------------------------
fs::path F3DConfigFileTools::GetUserConfigFileDirectory()
{
  std::string applicationName = "f3d";
  fs::path dirPath;
#if defined(_WIN32)
  const char* appData = std::getenv("APPDATA");
  if (!appData)
  {
    return fs::path();
  }
  dirPath = fs::path(appData);
#else
  // Implementing XDG specifications
  const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
  if (xdgConfigHome && strlen(xdgConfigHome) > 0)
  {
    dirPath = fs::path(xdgConfigHome);
  }
  else
  {
    const char* home = std::getenv("HOME");
    if (!home || strlen(home) == 0)
    {
      return fs::path();
    }
    dirPath = fs::path(home);
    dirPath /= ".config";
  }
#endif
  dirPath /= applicationName;
  return dirPath;
}

//----------------------------------------------------------------------------
fs::path F3DConfigFileTools::GetBinaryConfigFileDirectory()
{
  std::string execPath;
  fs::path dirPath;
  try
  {
#if defined(_WIN32)
    wchar_t wc[1024] = { 0 };
    GetModuleFileNameW(NULL, wc, 1024);
    std::wstring ws(wc);
    std::transform(
      ws.begin(), ws.end(), std::back_inserter(execPath), [](wchar_t c) { return (char)c; });
#else
#ifdef __APPLE__
    uint32_t size = 1024;
    char buffer[size];
    if (_NSGetExecutablePath(buffer, &size) != 0)
    {
      f3d::log::error("Executable is too long to recover path to configuration file");
      return fs::path();
    }
    execPath = buffer;
#else
    execPath = fs::canonical("/proc/self/exe").string();
#endif
#endif

    // transform path to exe to path to install
    // /install/bin/f3d -> /install
    dirPath = fs::canonical(fs::path(execPath)).parent_path().parent_path();

    // Add binary specific paths
#if F3D_MACOS_BUNDLE
    dirPath /= "Resources/configs";
#else
    dirPath /= "share/f3d/configs";
#endif
  }
  catch (const fs::filesystem_error&)
  {
    f3d::log::debug("Cannot recover binary configuration file directory: ", dirPath.string());
    return fs::path();
  }

  return dirPath;
}

//----------------------------------------------------------------------------
fs::path F3DConfigFileTools::GetConfigPath(const std::string& configSearch)
{
  fs::path configPath;
  std::vector<fs::path> dirsToCheck;
  try
  {
    dirsToCheck.emplace_back(F3DConfigFileTools::GetUserConfigFileDirectory());
#ifdef __APPLE__
    dirsToCheck.emplace_back("/usr/local/etc/f3d");
#endif
#ifdef __linux__
    dirsToCheck.emplace_back("/etc/f3d");
    dirsToCheck.emplace_back("/usr/share/f3d/configs");
#endif
    dirsToCheck.emplace_back(F3DConfigFileTools::GetBinaryConfigFileDirectory());

    for (const fs::path& dir : dirsToCheck)
    {
      if (dir.empty())
      {
        continue;
      }

      // If the config search is a stem, add extensions
      if (fs::path(configSearch).stem() == configSearch)
      {
        for (const std::string& ext : { std::string(".json"), std::string(".d") })
        {
          configPath = dir / (configSearch + ext);
          if (fs::exists(configPath))
          {
            return configPath;
          }
        }
      }
      else
      {
        // If not, use directly
        configPath = dir / (configSearch);
        if (fs::exists(configPath))
        {
          return configPath;
        }
      }
    }
    f3d::log::debug("No configuration file for \"", configSearch, "\" found");
    return fs::path();
  }
  catch (const fs::filesystem_error&)
  {
    f3d::log::error("Error recovering configuration file path: ", configPath.string());
    return fs::path();
  }
}
