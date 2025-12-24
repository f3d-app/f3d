#include "F3DSystemTools.h"

#include "log.h"
#include "utils.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <optional>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

namespace F3DSystemTools
{
//----------------------------------------------------------------------------
fs::path GetApplicationPath()
{
#if defined(_WIN32)
  std::array<wchar_t, 1024> wc{};
  if (GetModuleFileNameW(nullptr, wc.data(), 1024))
  {
    return fs::path(wc.data());
  }
  f3d::log::error("Cannot retrieve application path");
  return {};
#else
#ifdef __APPLE__
  uint32_t size = 1024;
  std::array<char, 1024> buffer;
  if (_NSGetExecutablePath(buffer.data(), &size) != 0)
  {
    f3d::log::error("Executable is too long to recover application path");
    return {};
  }
  return fs::path(buffer.data());
#else
  try
  {
#if defined(__FreeBSD__)
    return fs::canonical("/proc/curproc/file");
#else
    return fs::canonical("/proc/self/exe");
#endif
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("Cannot retrieve application path: ", ex.what());
    return {};
  }
#endif
#endif
}

std::vector<std::string> GetVectorEnvironnementVariable(const std::string& envVar)
{
  std::optional<std::string> envVal = f3d::utils::getEnv(envVar);
  if (!envVal.has_value() || envVal.value().empty())
  {
    return {};
  }

  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(envVal.value());

  // split path with OS separator (':' on Linux/macOS and ';' on Windows)
#ifdef _WIN32
  char delimiter = ';';
#else
  char delimiter = ':';
#endif

  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }

  return tokens;
}
}

//----------------------------------------------------------------------------
fs::path F3DSystemTools::GetUserScreenshotDirectory()
{
  fs::path dirPath;
#if defined(_WIN32)
  std::optional<std::string> appData =
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::PICTURES);
  if (!appData.has_value() || appData.value().empty())
  {
    return {};
  }
  dirPath = fs::path(appData.value());
#else
#if defined(__unix__)
  // Implementing XDG specifications
  std::optional<std::string> xdgPictures = f3d::utils::getEnv("XDG_PICTURES_DIR");
  if (xdgPictures.has_value() && !xdgPictures.value().empty())
  {
    dirPath = fs::path(xdgPictures.value());
  }
  else
#endif
  {
    std::optional<std::string> home = f3d::utils::getEnv("HOME");
    if (!home.has_value() || home.value().empty())
    {
      return {};
    }
    dirPath = fs::path(home.value());
  }
#endif
  return dirPath;
}

//----------------------------------------------------------------------------
fs::path F3DSystemTools::GetUserConfigFileDirectory()
{
  std::string applicationName = "f3d";
  fs::path dirPath;
#if defined(_WIN32)
  std::optional<std::string> appData =
    f3d::utils::getKnownFolder(f3d::utils::KnownFolder::ROAMINGAPPDATA);
  if (!appData.has_value() || appData.value().empty())
  {
    return {};
  }
  dirPath = fs::path(appData.value());
#else
#if defined(__unix__)
  // Implementing XDG specifications
  std::optional<std::string> xdgConfigHome = f3d::utils::getEnv("XDG_CONFIG_HOME");
  if (xdgConfigHome.has_value() && !xdgConfigHome.value().empty())
  {
    dirPath = fs::path(xdgConfigHome.value());
  }
  else
#endif
  {
    std::optional<std::string> home = f3d::utils::getEnv("HOME");
    if (!home.has_value() || home.value().empty())
    {
      return {};
    }
    dirPath = fs::path(home.value());
#if defined(__APPLE__)
    dirPath = dirPath / "Library" / "Application Support";
#elif defined(__unix__)
    dirPath /= ".config";
#endif
  }
#endif
  dirPath /= applicationName;
  return dirPath;
}

//----------------------------------------------------------------------------
fs::path F3DSystemTools::GetBinaryResourceDirectory()
{
  fs::path dirPath;
  try
  {
    dirPath = F3DSystemTools::GetApplicationPath();

    // transform path to exe to path to install
    // /install/bin/f3d -> /install
    dirPath = fs::canonical(dirPath).parent_path().parent_path();

    // Add binary specific paths
#if F3D_MACOS_BUNDLE
    dirPath /= "Resources";
#else
    dirPath /= "share/f3d";
#endif
  }
  catch (const fs::filesystem_error&)
  {
    f3d::log::debug("Cannot recover binary configuration file directory: ", dirPath.string());
    return {};
  }

  return dirPath;
}
