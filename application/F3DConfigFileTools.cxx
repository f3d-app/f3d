#include "F3DConfigFileTools.h"

#include "F3DSystemTools.h"

#include "nlohmann/json.hpp"

#include "log.h"
#include "utils.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

//----------------------------------------------------------------------------
std::vector<fs::path> F3DConfigFileTools::GetConfigPaths(const std::string& configSearch)
{
  std::vector<fs::path> paths;

  std::vector<fs::path> dirsToCheck = {

#ifdef __APPLE__
    "/usr/local/etc/f3d",
#endif
#if defined(__linux__) || defined(__FreeBSD__)
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

      std::vector<std::string> configNames;
      if (fs::path(configSearch).stem() == configSearch)
      {
        // If the config search is a stem, add extensions
        configNames.emplace_back(configSearch + ".json");
        configNames.emplace_back(configSearch + ".d");
      }
      else
      {
        // If not, use directly
        configNames.emplace_back(configSearch);
      }

      std::transform(configNames.begin(), configNames.end(), std::back_inserter(paths),
        [&dir](const auto& configName) { return dir / configName; });
    }
    catch (const fs::filesystem_error&)
    {
      f3d::log::error("Error recovering configuration file path");
    }
  }

  return paths;
}

//----------------------------------------------------------------------------
void F3DConfigFileTools::PrintConfigInfo(const std::vector<fs::path>& configPaths)
{
  if (!configPaths.empty())
  {
    f3d::log::info("Found available config path");
  }

  for (const fs::path& path : configPaths) 
  {
    std::error_code ec;
    const bool exists = fs::exists(path, ec);

    if (ec)
    {
      f3d::log::info("Error while checking config path: ", path, " : ", ec.message());
    }
    else if (exists)
    {
      f3d::log::info("Config file found: ", path);
    }
    else
    {
      f3d::log::info("Candidate config file not found: ", path);
    }
  }
}

//----------------------------------------------------------------------------
F3DConfigFileTools::ParsedConfigFiles F3DConfigFileTools::ReadConfigFiles(
  const std::string& userConfig)
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
    configPaths = F3DConfigFileTools::GetConfigPaths(configSearch);
  }
  else
  {
    // Collapse full path into an absolute path
    configPaths.emplace_back(f3d::utils::collapsePath(userConfig));
  }

  if (f3d::log::getVerboseLevel() == f3d::log::VerboseLevel::DEBUG)
  {
    F3DConfigFileTools::PrintConfigInfo(configPaths);
  }

  // Recover actual individual config file paths
  std::vector<fs::path> actualConfigFilePaths;
  bool explicitConfigMissing = false;
  for (auto configPath : configPaths)
  {
    try
    {
      if (!fs::exists(configPath))
      {
        if (!explicitConfigMissing && !userConfig.empty() && configSearch.empty())
        {
          f3d::log::error("Configuration file does not exist");
          explicitConfigMissing = true;
        }
        continue;
      }

      // Recover an absolute canonical path to config file
      configPath = fs::canonical(fs::path(configPath)).string();

      // Recover all config files if needed in directories
      if (fs::is_directory(configPath))
      {
        f3d::log::debug("Using config directory ", configPath.string());
        const size_t oldSize = actualConfigFilePaths.size();
        auto dirIter = fs::directory_iterator(configPath);
        std::copy(fs::begin(dirIter), fs::end(dirIter), std::back_inserter(actualConfigFilePaths));
        // directory_iterator is not ordered, enforce alphabetical ordering for the added files.
        std::sort(actualConfigFilePaths.begin() + oldSize, actualConfigFilePaths.end());
      }
      else
      {
        f3d::log::debug("Using config file ", configPath.string());
        actualConfigFilePaths.emplace_back(configPath);
      }
    }
    catch (const fs::filesystem_error& e)
    {
      f3d::log::error("Error while locating config path: ", e.what());
    }
  }

  // If we used a configSearch but did not find any, inform the user
  if (!configSearch.empty() && actualConfigFilePaths.empty())
  {
    f3d::log::info("Configuration file for \"", configSearch, "\" could not be found");
  }

  // Read config files
  F3DOptionsTools::OptionsEntries optionsEntries;
  F3DOptionsTools::OptionsEntries imperativeOptionsEntries;
  F3DConfigFileTools::BindingsEntries bindingsEntries;
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
    catch (const nlohmann::json::parse_error& ex)
    {
      f3d::log::error(
        "Unable to parse the configuration file ", configFilePath.string(), " , ignoring it");
      f3d::log::error(ex.what());
      continue;
    }

    try
    {
      // For each config block in the main array
      for (const auto& configBlock : json)
      {
        // Recover match_type if any
        std::string matchType;
        try
        {
          matchType = configBlock.at("match-type");
          if (matchType != "regex" && matchType != "glob" && matchType != "exact")
          {
            f3d::log::warn("A config block in config file ", configFilePath.string(),
              " has an unknown match-type (", matchType, "), defaulting to regex");
            matchType = "regex";
          }
        }
        catch (nlohmann::json::out_of_range&)
        {
          // Not match_type defined, use regex
          matchType = "regex";
        }

        // Recover match if any
        std::string match;
        try
        {
          match = configBlock.at("match");
        }
        catch (nlohmann::json::out_of_range&)
        {
          // No match defined, use a catch all regex
          if (matchType != "regex")
          {
            f3d::log::warn("A config block in config file ", configFilePath.string(),
              " has match-type ", matchType, " but no match expression, using a catch-all regex");
            matchType = "regex";
          }
          match = ".*";
        }

        // Recover options if any
        nlohmann::ordered_json optionsBlock;
        try
        {
          optionsBlock = configBlock.at("options");
        }
        catch (nlohmann::json::out_of_range&)
        {
        }

        if (!optionsBlock.empty())
        {
          // Add each options config entry into an option dict
          F3DOptionsTools::OptionsDict entry;
          F3DOptionsTools::OptionsDict imperativeEntry;
          for (const auto& item : optionsBlock.items())
          {
            // Individual item can be imperative, store in the right dict
            std::string key = item.key();
            std::reference_wrapper<F3DOptionsTools::OptionsDict> localEntry = std::ref(entry);
            if (!key.empty() && key[0] == '!')
            {
              localEntry = std::ref(imperativeEntry);
              key = key.substr(1);
            }

            if (item.value().is_number() || item.value().is_boolean())
            {
              localEntry.get()[key] = nlohmann::to_string(item.value());
            }
            else if (item.value().is_string())
            {
              localEntry.get()[key] = item.value().get<std::string>();
            }
            else
            {
              f3d::log::error(key, " from ", configFilePath.string(),
                " must be a string, a boolean or a number, ignoring entry");
              continue;
            }
          }

          // Emplace the option dicts for that pattern match into the config entries vector
          if (!entry.empty())
          {
            optionsEntries.emplace_back(entry, configFilePath.string(), matchType, match);
          }
          if (!imperativeEntry.empty())
          {
            // The path is only used for logging purpose, store the imperative information inside
            imperativeOptionsEntries.emplace_back(
              imperativeEntry, configFilePath.string() + " (imperative)", matchType, match);
          }
        }

        // Recover bindings if any
        nlohmann::ordered_json bindingsBlock;
        try
        {
          bindingsBlock = configBlock.at("bindings");
        }
        catch (nlohmann::json::out_of_range&)
        {
        }

        if (!bindingsBlock.empty())
        {
          // Add each binding config entry
          F3DConfigFileTools::BindingsVector bindingEntry;
          for (const auto& item : bindingsBlock.items())
          {
            if (item.value().is_string())
            {
              bindingEntry.emplace_back(std::make_pair(
                item.key(), std::vector<std::string>{ item.value().get<std::string>() }));
            }
            else if (item.value().is_array())
            {
              // Do not check before we look for simplicity sake
              bindingEntry.emplace_back(
                std::make_pair(item.key(), item.value().get<std::vector<std::string>>()));
            }
            else
            {
              f3d::log::error(item.key(), " from ", configFilePath.string(),
                " must be a string or an array of string, ignoring binding entry");
              continue;
            }
          }

          // Emplace the config dict for that pattern match into the binding entries vector
          bindingsEntries.emplace_back(bindingEntry, configFilePath.string(), matchType, match);
        }

        if (optionsBlock.empty() && bindingsBlock.empty())
        {
          // To help users figure out issues with configuration files
          f3d::log::warn("A config block in config file: ", configFilePath.string(),
            " does not contains options nor bindings, ignoring block");
        }
      }
    }
    catch (const nlohmann::json::type_error& ex)
    {
      f3d::log::warn("Error processing config file: ", configFilePath.string(),
        ", configuration may be incorrect");
      f3d::log::error(ex.what());
    }
  }
  return F3DConfigFileTools::ParsedConfigFiles{ std::move(optionsEntries),
    std::move(imperativeOptionsEntries), std::move(bindingsEntries), std::move(configPaths) };
}
