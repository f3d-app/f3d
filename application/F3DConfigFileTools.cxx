#include "F3DConfigFileTools.h"

#include "F3DConfig.h"
#include "F3DSystemTools.h"

#include "log.h"

#include <cstring>
#include <filesystem>
#include <vector>

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
  fs::path dirPath;
  try
  {
    dirPath = F3DSystemTools::GetApplicationPath();

    // transform path to exe to path to install
    // /install/bin/f3d -> /install
    dirPath = fs::canonical(dirPath).parent_path().parent_path();

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
        for (const std::string& ext : { ".json", ".d" })
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
