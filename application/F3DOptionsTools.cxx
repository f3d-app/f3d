#include "F3DOptionsTools.h"

#include "F3DCLIOptions.h"
#include "F3DConfig.h"
#include "F3DException.h"
#include "F3DPluginsTools.h"
#include "F3DSystemTools.h"

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "utils.h"

#include <cassert>
#include <filesystem>
#include <iomanip>
#include <numeric>
#include <set>
#include <sstream>

#include "cxxopts.hpp"

namespace fs = std::filesystem;

namespace
{
/**
 * True boolean options need to be filtered out in ParseCLIOptions
 * Also filter out special options like `define` and `reset`
 * This is the easiest, compile time way to do it
 */
constexpr std::array CLIBooleans = { "version", "help", "list-readers", "scan-plugins",
  "list-rendering-backends", "define", "reset" };

//----------------------------------------------------------------------------
/**
 * Collapse a longName and shortName into the expected syntax of cxxopts for name declaration
 */
std::string CollapseName(const std::string_view& longName, const std::string_view& shortName)
{
  std::stringstream ss;
  if (shortName != "")
  {
    ss << shortName << ",";
  }
  ss << longName;
  return ss.str();
}

//----------------------------------------------------------------------------
void PrintHelp(const std::string& execName, const cxxopts::Options& cxxOptions)
{
  const std::array<std::pair<std::string, std::string>, 4> examples = { {
    { execName + " file.vtu -xtgans",
      "View a unstructured mesh in a typical nice looking sciviz style" },
    { execName + " file.glb -tuqap --hdri-file=file.hdr --hdri-ambient --hdri-skybox",
      "View a gltf file in a realistic environment" },
    { execName + " file.ply -so --point-size=0 --coloring-component=-2",
      "View a point cloud file with direct scalars rendering" },
    { execName + " folder", "View all files in folder" },
  } };

  f3d::log::setUseColoring(false);
  std::vector<std::string> orderedCLIGroupNames(F3D::CLIOptions.size());
  std::transform(F3D::CLIOptions.cbegin(), F3D::CLIOptions.cend(), orderedCLIGroupNames.begin(),
    [](const F3D::CLIGroup& cliGroup) { return cliGroup.GroupName; });
  f3d::log::info(cxxOptions.help(orderedCLIGroupNames));
  f3d::log::info("\nExamples:");
  for (const auto& [cmd, desc] : examples)
  {
    F3DOptionsTools::PrintHelpPair(cmd, desc, 50);
  }
  f3d::log::info("\nReport bugs to https://github.com/f3d-app/f3d/issues");
  f3d::log::setUseColoring(true);
}

//----------------------------------------------------------------------------
void PrintPluginsScan()
{
#if F3D_MACOS_BUNDLE
  f3d::log::error("option not supported with the macOS bundle");
#else
  auto appPath = F3DSystemTools::GetApplicationPath();
  appPath = appPath.parent_path().parent_path();

  appPath /= "share/f3d/plugins";

  auto plugins = f3d::engine::getPluginsList(appPath);

  f3d::log::info("Found ", plugins.size(), " plugins:");

  for (const std::string& p : plugins)
  {
    f3d::log::info(" - ", p);
  }
#endif
}

//----------------------------------------------------------------------------
void PrintVersion()
{
  f3d::log::setUseColoring(false);
  f3d::log::info(F3D::AppName + " " + F3D::AppVersion + "\n");
  f3d::log::info(F3D::AppTitle);
  auto libInfo = f3d::engine::getLibInfo();
  f3d::log::info("Version: " + libInfo.VersionFull + ".");
  f3d::log::info("Build date: " + libInfo.BuildDate + ".");
  f3d::log::info("Build system: " + libInfo.BuildSystem + ".");
  f3d::log::info("Compiler: " + libInfo.Compiler + ".");
  for (const auto& [name, enabled] : libInfo.Modules)
  {
    f3d::log::info("Module " + name + ": " + (enabled ? "ON." : "OFF."));
  }
  f3d::log::info("VTK version: " + libInfo.VTKVersion + ".");
  for (const auto& cr : libInfo.Copyrights)
  {
    f3d::log::info("Copyright (C) " + cr + ".");
  }
  f3d::log::info("License " + libInfo.License + ".");
  f3d::log::setUseColoring(true);
}

//----------------------------------------------------------------------------
void PrintRenderingBackendList()
{
  auto backends = f3d::engine::getRenderingBackendList();

  f3d::log::setUseColoring(false);
  f3d::log::info("Rendering backends:");
  for (const auto& [name, available] : backends)
  {
    f3d::log::info(name + ": " + (available ? "available" : "unavailable"));
  }
}

//----------------------------------------------------------------------------
void PrintReadersList()
{
  size_t nameColSize = 0;
  size_t extsColSize = 0;
  size_t mimeColSize = 0;
  size_t descColSize = 0;
  size_t plugColSize = 0;
  constexpr std::string_view streamStr = "Supports Stream";
  size_t streamColSize = streamStr.size();

  std::vector<f3d::engine::readerInformation> readersInfo = f3d::engine::getReadersInfo();
  if (readersInfo.empty())
  {
    f3d::log::warn("No registered reader found!");
    return;
  }
  // Compute the size of the 5 columns
  for (const auto& reader : readersInfo)
  {
    // There is at most one MIME type by extension
    assert(reader.Extensions.size() >= reader.MimeTypes.size());

    nameColSize = std::max(nameColSize, reader.Name.length());
    descColSize = std::max(descColSize, reader.Description.length());
    plugColSize = std::max(plugColSize, reader.PluginName.length());

    extsColSize = std::accumulate(reader.Extensions.cbegin(), reader.Extensions.cend(), extsColSize,
      [](size_t size, const auto& ext) { return std::max(size, ext.length()); });
    mimeColSize = std::accumulate(reader.MimeTypes.cbegin(), reader.MimeTypes.cend(), mimeColSize,
      [](size_t size, const auto& mime) { return std::max(size, mime.length()); });
  }
  const size_t colGap = 4;
  nameColSize += colGap;
  extsColSize += colGap;
  mimeColSize += colGap;
  descColSize += colGap;
  plugColSize += colGap;
  streamColSize += colGap;

  std::string separator = std::string(
    nameColSize + extsColSize + descColSize + mimeColSize + plugColSize + streamColSize - colGap,
    '-');

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(plugColSize) << "Plugin"
             << std::setw(descColSize) << "Description" << std::setw(streamColSize) << streamStr
             << std::setw(extsColSize) << "Exts" << std::setw(mimeColSize - colGap) << "Mime-types";
  f3d::log::info(headerLine.str());
  f3d::log::info(separator);

  for (const auto& reader : readersInfo)
  {
    for (size_t i = 0; i < reader.Extensions.size(); i++)
    {
      std::stringstream readerLine;
      readerLine << std::left;
      readerLine << std::setw(nameColSize) << (i == 0 ? reader.Name : "");
      readerLine << std::setw(plugColSize) << (i == 0 ? reader.PluginName : "");
      readerLine << std::setw(descColSize) << (i == 0 ? reader.Description : "");
      readerLine << std::setw(streamColSize)
                 << (i == 0 ? (reader.SupportsStream ? "YES" : "NO") : "");
      readerLine << std::setw(extsColSize)
                 << (i < reader.Extensions.size() ? reader.Extensions[i] : "");
      readerLine << std::setw(mimeColSize - colGap)
                 << (i < reader.MimeTypes.size() ? reader.MimeTypes[i] : "");

      f3d::log::info(readerLine.str());
    }
  }
}
}

//----------------------------------------------------------------------------
std::pair<std::string, int> F3DOptionsTools::GetClosestOption(
  const std::string& option, bool checkLibAndReaders)
{
  std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };
  auto checkDistance =
    [](const std::string& key, const std::string& name, std::pair<std::string, int>& ref)
  {
    int distance = f3d::utils::textDistance(key, name);
    if (distance < ref.second)
    {
      ref = { key, distance };
    }
  };

  // Check positional arg `--input`
  checkDistance("input", option, ret);

  // Check true boolean options
  for (std::string_view key : ::CLIBooleans)
  {
    checkDistance(std::string(key), option, ret);
  }

  // Check cli names in app options
  for (const auto& [key, value] : F3DOptionsTools::DefaultAppOptions)
  {
    checkDistance(key, option, ret);
  }

  // Check cli names in custom mapping options
  for (const auto& [key, value] : F3DOptionsTools::CustomMappingOptions)
  {
    checkDistance(std::string(key), option, ret);
  }

  // Check cli names for libf3d options
  for (const auto& [key, value] : F3DOptionsTools::LibOptionsNames)
  {
    checkDistance(std::string(key), option, ret);
  }

  // Check libf3d and reader option names
  if (checkLibAndReaders)
  {
    for (const std::string& key : f3d::options::getAllNames())
    {
      checkDistance(key, option, ret);
    }

    for (const std::string& key : f3d::engine::getAllReaderOptionNames())
    {
      checkDistance(key, option, ret);
    }
  }
  return ret;
}

//----------------------------------------------------------------------------
F3DOptionsTools::OptionsDict F3DOptionsTools::ParseCLIOptions(
  int argc, char** argv, std::vector<std::string>& positionals)
{
  std::string execName = argc > 0 && argv[0][0] ? fs::path(argv[0]).filename().string() : "f3d";

  // cxxopts values need to live somewhere until parsing is done
  std::vector<std::shared_ptr<cxxopts::Value>> cxxoptsValues;
  auto cxxoptsInputPositionals = cxxopts::value<std::vector<std::string>>(positionals);

  std::vector<std::string> defines;
  auto cxxoptsDefines = cxxopts::value<std::vector<std::string>>(defines);

  std::vector<std::string> resets;
  auto cxxoptsResets = cxxopts::value<std::vector<std::string>>(resets);

  try
  {
    cxxopts::Options cxxOptions(execName, F3D::AppTitle);
    cxxOptions.custom_help("[OPTIONS...]");

    for (const F3D::CLIGroup& optionGroup : F3D::CLIOptions)
    {
      auto group = cxxOptions.add_options(std::string(optionGroup.GroupName));

      // Positional option, `--input` require a custom implementation
      if (std::string(optionGroup.GroupName) == "Applicative")
      {
        group("input", "Input files", cxxoptsInputPositionals, "<files>");
        group("D,define", "Define libf3d options", cxxoptsDefines, "libf3d.option=value");
        group("R,reset", "Reset libf3d options", cxxoptsResets, "libf3d.option");
      }

      // Add each option to cxxopts
      for (const F3D::CLIOption& cliOption : optionGroup.Options)
      {
        if (cliOption.ValueHelper.empty())
        {
          // No ValueHelper means its a true boolean option like `--help` or `--version`
          group(::CollapseName(cliOption.LongName, cliOption.ShortName),
            std::string(cliOption.HelpText));
        }
        else
        {
          // Add the default value to the help text if any
          std::string defaultValue;
          std::string helpText(cliOption.HelpText);
          std::string longName(cliOption.LongName);

          // Recover default value from app options
          auto appIter = F3DOptionsTools::DefaultAppOptions.find(longName);
          if (appIter != F3DOptionsTools::DefaultAppOptions.end())
          {
            defaultValue = appIter->second;
          }
          else
          {
            auto customIter = F3DOptionsTools::CustomMappingOptions.find(longName);
            if (customIter != F3DOptionsTools::CustomMappingOptions.end())
            {
              defaultValue = customIter->second;
            }
            else
            {
              // Recover default value from lib options
              auto libIter = F3DOptionsTools::LibOptionsNames.find(cliOption.LongName);
              if (libIter != F3DOptionsTools::LibOptionsNames.end())
              {
                f3d::options opt;
                std::string name = std::string(libIter->second);
                // let default value empty for unset options
                defaultValue = opt.hasValue(name) ? opt.getAsString(name) : "";
              }
            }
          }

          // Add default value to the help text directly
          // Do not add it as a default value in cxxopts
          // As it would add it to the parseResults
          // which we do not want
          if (!defaultValue.empty())
          {
            helpText += " (default: " + defaultValue + ")";
          }

          // Recover the implicit value and set it if any
          cxxoptsValues.emplace_back(cxxopts::value<std::string>());
          auto& val = cxxoptsValues.back();
          if (!cliOption.ImplicitValue.empty())
          {
            val->implicit_value(std::string(cliOption.ImplicitValue));
          }

          // Add the cxxopts option
          group(::CollapseName(cliOption.LongName, cliOption.ShortName), helpText, val,
            std::string(cliOption.ValueHelper));
        }
      }
    }

    // Parse using cxxopts
    cxxOptions.allow_unrecognised_options();
    cxxOptions.positional_help("file1 file2 ...");
    cxxOptions.parse_positional({ "input" });
    cxxOptions.show_positional_help();
    auto result = cxxOptions.parse(argc, argv);

    // Check boolean options and log them if any
    if (result.count("help") > 0)
    {
      ::PrintHelp(execName, cxxOptions);
      throw F3DExNoProcess("help requested");
    }
    if (result.count("version") > 0)
    {
      ::PrintVersion();
      throw F3DExNoProcess("version requested");
    }
    if (result.count("list-rendering-backends") > 0)
    {
      ::PrintRenderingBackendList();
      throw F3DExNoProcess("rendering backend list requested");
    }
    if (result.count("scan-plugins") > 0)
    {
      ::PrintPluginsScan();
      throw F3DExNoProcess("scan plugins requested");
    }
    if (result.count("list-readers") > 0)
    {
      // `--list-readers` needs plugin to be loaded to be useful
      // Load them manually
      std::vector<std::string> plugins;
      if (result.count("load-plugins") > 0)
      {
        plugins =
          f3d::options::parse<std::vector<std::string>>(result["load-plugins"].as<std::string>());
      }
      F3DPluginsTools::LoadPlugins(plugins);
      ::PrintReadersList();
      throw F3DExNoProcess("reader list requested");
    }

    // Check for unknown options and log them
    auto unmatched = result.unmatched();
    bool foundUnknownOption = false;
    for (const std::string& unknownOption : unmatched)
    {
      f3d::log::error("Unknown option '", unknownOption, "'");
      foundUnknownOption = true;

      // check if it's a long option
      if (unknownOption.substr(0, 2) == "--")
      {
        const size_t equalPos = unknownOption.find('=');

        // remove "--" and everything after the first "=" (if any)
        const std::string unknownName =
          unknownOption.substr(2, equalPos != std::string::npos ? equalPos - 2 : equalPos);

        auto [closestName, dist] = F3DOptionsTools::GetClosestOption(unknownName);
        const std::string closestOption = equalPos == std::string::npos
          ? closestName
          : closestName + unknownOption.substr(equalPos);

        f3d::log::error("Did you mean '--", closestOption, "'?");
      }
    }
    if (foundUnknownOption)
    {
      throw F3DExFailure("unknown options");
    }

    // Add each CLI options into a vector of string/string and return it
    F3DOptionsTools::OptionsDict cliOptionsDict;
    for (const auto& res : result)
    {
      // Discard boolean option like `--version` or `--help`
      if (std::find(::CLIBooleans.begin(), ::CLIBooleans.end(), res.key()) == ::CLIBooleans.end())
      {
        cliOptionsDict[res.key()] = res.value();
      }
    }

    // Handle defines and add them as proper options
    for (const std::string& define : defines)
    {
      std::string::size_type sepIdx = define.find_first_of('=');
      if (sepIdx == std::string::npos)
      {
        f3d::log::warn("Could not parse a define '", define, "'");
        continue;
      }
      cliOptionsDict[define.substr(0, sepIdx)] = define.substr(sepIdx + 1);
    }

    // Handles reset using the dedicated syntax
    for (const std::string& reset : resets)
    {
      cliOptionsDict["reset-" + reset] = "";
    }

    return cliOptionsDict;
  }
  catch (const cxxopts::exceptions::exception& ex)
  {
    f3d::log::error("Error parsing command line arguments: ", ex.what());
    throw F3DExFailure("Could not parse command line arguments");
  }
}

//----------------------------------------------------------------------------
void F3DOptionsTools::PrintHelpPair(
  std::string_view key, std::string_view help, int keyWidth, int helpWidth)
{
  std::stringstream ss;
  ss << "  " << std::left << std::setw(keyWidth) << key;
  if (key.size() > static_cast<size_t>(keyWidth))
  {
    ss << "\n  " << std::setw(keyWidth) << " ";
  }
  ss << " " << std::setw(helpWidth) << help;
  f3d::log::info(ss.str());
}

//----------------------------------------------------------------------------
std::vector<std::pair<std::string, std::string>> F3DOptionsTools::ConvertToLibf3dOptions(
  const std::string& key, const std::string& value)
{
  std::vector<std::pair<std::string, std::string>> libf3dOptions;

  // Simple one-to-one case
  auto libf3dIter = F3DOptionsTools::LibOptionsNames.find(key);
  if (libf3dIter != F3DOptionsTools::LibOptionsNames.end())
  {
    libf3dOptions.emplace_back(std::make_pair(libf3dIter->second, value));
  }

  // anti-aliasing is handled in two options in the lib
  else if (key == "anti-aliasing")
  {
    if (value != "none")
    {
      // Handle deprecated boolean option
      bool deprecatedBooleanOption;
      if (F3DOptionsTools::Parse(value, deprecatedBooleanOption))
      {
        f3d::log::warn("--anti-aliasing is a now a string, please specify the type of "
                       "anti-aliasing or use the implicit default");
        libf3dOptions.emplace_back(std::make_pair("render.effect.antialiasing.enable", value));
      }
      else
      {
        libf3dOptions.emplace_back(std::make_pair("render.effect.antialiasing.enable", "true"));
        libf3dOptions.emplace_back(std::make_pair("render.effect.antialiasing.mode", value));
      }
    }
    else
    {
      libf3dOptions.emplace_back(std::make_pair("render.effect.antialiasing.enable", "false"));
    }
  }

  // handle deprecated anti-aliasing option
  else if (key == "anti-aliasing-mode")
  {
    f3d::log::warn("--anti-aliasing-mode is deprecated");
    libf3dOptions.emplace_back(std::make_pair("render.effect.antialiasing.mode", value));
  }

  // blending is handled in two options in the lib
  else if (key == "blending")
  {
    if (value != "none")
    {
      libf3dOptions.emplace_back(std::make_pair("render.effect.blending.enable", "true"));
      libf3dOptions.emplace_back(std::make_pair("render.effect.blending.mode", value));
    }
    else
    {
      libf3dOptions.emplace_back(std::make_pair("render.effect.blending.enable", "false"));
    }
  }

  // handle deprecated translucency support
  else if (key == "translucency-support")
  {
    f3d::log::warn("--translucency-support is deprecated, please use --blending instead");
    libf3dOptions.emplace_back(std::make_pair("render.effect.blending.enable", value));
  }

  // point sprites is handled in two options in the lib
  else if (key == "point-sprites")
  {
    if (value != "none")
    {
      // Handle deprecated boolean option
      bool deprecatedBooleanOption;
      if (F3DOptionsTools::Parse(value, deprecatedBooleanOption))
      {
        f3d::log::warn("--point-sprites is a now a string, please specify the type of "
                       "point sprites to use or use the implicit default");
        libf3dOptions.emplace_back(std::make_pair("model.point_sprites.enable", value));
      }
      else
      {
        libf3dOptions.emplace_back(std::make_pair("model.point_sprites.enable", "true"));
        libf3dOptions.emplace_back(std::make_pair("model.point_sprites.type", value));
      }
    }
    else
    {
      libf3dOptions.emplace_back(std::make_pair("model.point_sprites.enable", "false"));
    }
  }

  // handle deprecated point-sprites-type option
  else if (key == "point-sprites-type")
  {
    f3d::log::warn("--point-sprites-type is deprecated");
    libf3dOptions.emplace_back(std::make_pair("model.point_sprites.mode", value));
  }

  // handle deprecated interaction-trackball option
  else if (key == "interaction-trackball")
  {
    f3d::log::warn("--interaction-trackball is deprecated, please use --interaction-style=trackball instead");
    bool trackball;
    if (F3DOptionsTools::Parse(value, trackball) && trackball)
    {
      libf3dOptions.emplace_back(std::make_pair("interactor.style", "trackball"));
    }
  }

  else
  {
    // If nothing to convert, just return the input
    libf3dOptions.emplace_back(std::make_pair(key, value));
  }

  return libf3dOptions;
}
