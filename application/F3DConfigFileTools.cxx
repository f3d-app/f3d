#include "F3DConfigFileTools.h"

#include "F3DSystemTools.h"

#include "nlohmann/json.hpp"

#include "log.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <vector>

namespace fs = std::filesystem;

namespace
{
//----------------------------------------------------------------------------
/**
 * Recover a OS-specific vector of potential config file directories
 */
std::vector<fs::path> GetConfigPaths(const std::string& configSearch)
{
  std::vector<std::filesystem::path> paths;

  fs::path configPath;
  std::vector<fs::path> dirsToCheck = {

#ifdef __APPLE__
    "/usr/local/etc/f3d",
#endif
#ifdef __linux__
    "/etc/f3d",
    "/usr/share/f3d/configs",
#endif
    F3DSystemTools::GetBinaryResourceDirectory() / "configs",
    F3DSystemTools::GetUserConfigFileDirectory(),
  };

  for (const fs::path& dir : dirsToCheck)
  {
    try
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
            paths.emplace_back(configPath);
          }
        }
      }
      else
      {
        // If not, use directly
        configPath = dir / (configSearch);
        if (fs::exists(configPath))
        {
          paths.emplace_back(configPath);
        }
      }
    }
    catch (const fs::filesystem_error&)
    {
      f3d::log::error("Error recovering configuration file path: ", configPath.string());
    }
  }

  return paths;
}
}

//----------------------------------------------------------------------------
F3DOptionsTools::OptionsEntries F3DConfigFileTools::ReadConfigFiles(const std::string& userConfig)
{
  // Default config directory name
  std::string configSearch = "config";
  if (!userConfig.empty())
  {
    // Check if provided userConfig is a full path
    auto path = fs::path(userConfig);
    if (path.stem() == userConfig || path.filename() == userConfig)
    {
      // Only a stem or a filename, use provided userConfig as configSearch
      configSearch = userConfig;
    }
    else
    {
      // Assume its a full path and use as is, not searching for config files
      configSearch = "";
    }
  }

  // Recover config paths to search for config files
  std::vector<fs::path> configPaths;
  if (!configSearch.empty())
  {
    for (const auto& path : ::GetConfigPaths(configSearch))
    {
      configPaths.emplace_back(path);
    }
  }
  else
  {
    configPaths.emplace_back(userConfig);
  }

  // Recover actual individual config file paths
  std::set<fs::path> actualConfigFilePaths;
  for (auto configPath : configPaths)
  {
    // Recover an absolute canonical path to config file
    try
    {
      configPath = fs::canonical(fs::path(configPath)).string();
    }
    catch (const fs::filesystem_error&)
    {
      f3d::log::error("Configuration file does not exist: ", configPath.string(), " , ignoring it");
      continue;
    }

    // Recover all config files if needed in directories
    if (fs::is_directory(configPath))
    {
      f3d::log::debug("Using config directory ", configPath.string());
      for (auto& entry : std::filesystem::directory_iterator(configPath))
      {
        actualConfigFilePaths.emplace(entry);
      }
    }
    else
    {
      f3d::log::debug("Using config file ", configPath.string());
      actualConfigFilePaths.emplace(configPath);
    }
  }

  // If we used a configSearch but did not find any, warn the user
  if (!configSearch.empty() && actualConfigFilePaths.empty())
  {
    f3d::log::warn("Configuration file for \"", configSearch, "\" could not be found");
  }

  // Read config files
  F3DOptionsTools::OptionsEntries confEntries;
  for (const auto& configFilePath : actualConfigFilePaths)
  {
    std::ifstream file(configFilePath);
    if (!file.is_open())
    {
      // Cannot be tested
      f3d::log::warn(
        "Unable to open the configuration file: ", configFilePath.string(), " , ignoring it");
      continue;
    }

    // Read the file into a json
    nlohmann::ordered_json json;
    try
    {
      file >> json;
    }
    catch (const std::exception& ex)
    {
      f3d::log::error(
        "Unable to parse the configuration file ", configFilePath.string(), " , ignoring it");
      f3d::log::error(ex.what());
      continue;
    }

    // For each config block in the main array
    for (const auto& configBlock : json)
    {
      // Recover match if any
      std::string match;
      try
      {
        match = configBlock.at("match");
      }
      catch (nlohmann::json::exception&)
      {
        // No match defined, use a catch all regex
        match = ".*";
      }

      // Recover options if any
      nlohmann::ordered_json optionsBlock;
      try
      {
        optionsBlock = configBlock.at("options");
      }
      catch (nlohmann::json::exception&)
      {
      }

      // Add each options config entry into an option dict
      F3DOptionsTools::OptionsDict entry;
      for (const auto& item : optionsBlock.items())
      {
        if (item.value().is_number() || item.value().is_boolean())
        {
          entry[item.key()] = nlohmann::to_string(item.value());
        }
        else if (item.value().is_string())
        {
          entry[item.key()] = item.value().get<std::string>();
        }
        else
        {
          f3d::log::error(item.key(), " from ", configFilePath.string(),
            " must be a string, a boolean or a number, ignoring entry");
          continue;
        }
      }

      // Emplace the option dict for that pattern match into the config entries vector
      confEntries.emplace_back(entry, configFilePath, match);
    }
  }
  return confEntries;
}
