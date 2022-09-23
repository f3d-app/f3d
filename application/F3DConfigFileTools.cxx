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

//----------------------------------------------------------------------------
std::string F3DConfigFileTools::GetUserConfigFileDirectory()
{
  std::string applicationName = "f3d";
  std::filesystem::path dirPath;
#if defined(_WIN32)
  const char* appData = std::getenv("APPDATA");
  if (!appData)
  {
    return std::string();
  }
  dirPath = std::filesystem::path(appData);
#else
  // Implementing XDG specifications
  const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
  if (xdgConfigHome && strlen(xdgConfigHome) > 0)
  {
    dirPath = std::filesystem::path(xdgConfigHome);
  }
  else
  {
    const char* home = std::getenv("HOME");
    if (!home || strlen(home) == 0)
    {
      return std::string();
    }
    dirPath = std::filesystem::path(home);
    dirPath /= ".config";
  }
#endif
  dirPath /= applicationName;
  return dirPath.string();
}

//----------------------------------------------------------------------------
std::string F3DConfigFileTools::GetBinaryConfigFileDirectory()
{
  std::string execPath;
  std::filesystem::path dirPath;
  std::string errorMsg, programFilePath;
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
      return std::string();
    }
    execPath = buffer;
#else
    execPath = std::filesystem::canonical("/proc/self/exe").string();
#endif
#endif

    // transform path to exe to path to install
    // /install/bin/f3d -> /install
    dirPath =
      std::filesystem::canonical(std::filesystem::path(execPath)).parent_path().parent_path();

    // Add platform specific paths
#if F3D_MACOS_BUNDLE
    dirPath /= "Resources";
#endif
#ifdef __linux__
    dirPath /= "share/f3d";
#endif
  }
  catch (const std::filesystem::filesystem_error&)
  {
    f3d::log::debug("Cannot recover binary configuration file directory: ", dirPath.string());
    return std::string();
  }

  return dirPath.string();
}

//----------------------------------------------------------------------------
std::string F3DConfigFileTools::GetConfigFilePath(const std::string& filename)
{
  std::filesystem::path filePath;
  std::vector<std::string> dirsToCheck;
  try
  {
    dirsToCheck.emplace_back(F3DConfigFileTools::GetUserConfigFileDirectory());
#ifdef __APPLE__
    dirsToCheck.emplace_back("/usr/local/etc/f3d");
#endif
#ifdef __linux__
    dirsToCheck.emplace_back("/etc/f3d");
    dirsToCheck.emplace_back("/usr/share/f3d");
#endif
    dirsToCheck.emplace_back(F3DConfigFileTools::GetBinaryConfigFileDirectory());

    for (std::string dir : dirsToCheck)
    {
      if (!dir.empty())
      {
        filePath = std::filesystem::path(dir) / filename;
        if (std::filesystem::exists(filePath))
        {
          return filePath.string();
        }
      }
    }
    f3d::log::debug("No configuration file (\"", filename, "\") found");
    return std::string();
  }
  catch (const std::filesystem::filesystem_error&)
  {
    f3d::log::error("Error recovering configuration file path: ", filePath.string());
    return std::string();
  }
}
