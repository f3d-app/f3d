#include "F3DOptionsTools.h"

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
#include <set>
#include <sstream>

#include "cxxopts.hpp"

namespace fs = std::filesystem;

namespace
{

/**
 * A struct to configure a complete cxxopts CLI options
 * F3D uses exclusively string options except for a few true boolean option (eg: `--version`)
 * LongName: The long name, eg: `axis`
 * ShortName: The short name, eg: `x`
 * HelpText: The help text, for display only when using `--help`
 * ValueHelper: Used in help but also to discrimate between boolean option or not
 * ImplicitValue: The implicit value when option is provided without a value
 */
struct CLIOption
{
  std::string_view LongName;
  std::string_view ShortName;
  std::string_view HelpText;
  std::string_view ValueHelper;
  std::string_view ImplicitValue;
};

/**
 * A struct to group option into categories
 */
struct CLIGroup
{
  std::string_view GroupName;
  std::vector<CLIOption> Options;
};

/**
 * Declaration of all F3D CLI options except `--input` using above structs
 * Order of groups matters in the context of `--help`
 */
// clang-format off
#if F3D_MODULE_RAYTRACING
static inline const std::array<CLIGroup, 9> CLIOptions = {{
#else
static inline const std::array<CLIGroup, 8> CLIOptions = {{
#endif
  { "Applicative",
    { { "output", "", "Render to file", "<png file>", "" },
      { "no-background", "", "No background when render to file", "<bool>", "1" },
      { "help", "h", "Print help", "", "" }, { "version", "", "Print version details", "", "" },
      { "readers-list", "", "Print the list of readers", "", "" },
      { "config", "", "Specify the configuration file to use. absolute/relative path or filename/filestem to search in configuration file locations", "<filePath/filename/fileStem>", "" },
      { "dry-run", "", "Do not read the configuration file", "<bool>", "1" },
      { "no-render", "", "Do not read the configuration file", "<bool>", "1" },
      { "max-size", "", "Maximum size in Mib of a file to load, negative value means unlimited", "<size in Mib>", "" },
      { "watch", "", "Watch current file and automatically reload it whenever it is modified on disk", "<bool>", "1" },
      { "load-plugins", "", "List of plugins to load separated with a comma", "<paths or names>", "" },
      { "scan-plugins", "", "Scan standard directories for plugins and display available plugins (result can be incomplete)", "", "" },
      { "screenshot-filename", "", "Screenshot filename", "<filename>", "" } } },
  { "General",
    { { "verbose", "", "Set verbose level, providing more information about the loaded data in the console output", "{debug, info, warning, error, quiet}", "debug" },
      { "progress", "", "Show loading progress bar", "<bool>", "1" },
      { "animation-progress", "", "Show animation progress bar", "<bool>", "1" },
      { "geometry-only", "", "Do not read materials, cameras and lights from file", "<bool>", "1" },
      { "group-geometries", "", "When opening multiple files, show them all in the same scene. Force geometry-only. The configuration file for the first file will be loaded.", "<bool>", "1" },
      { "up", "", "Up direction", "{-X, +X, -Y, +Y, -Z, +Z}", "" },
      { "axis", "x", "Show axes", "<bool>", "1" }, { "grid", "g", "Show grid", "<bool>", "1" },
      { "grid-absolute", "", "Position grid at the absolute origin instead of below the model", "<bool>", "1" },
      { "grid-unit", "", "Size of grid unit square, set to a non-positive value for automatic computation", "<value>", "" },
      { "grid-subdivisions", "", "Number of grid subdivisions", "<value>", "" },
      { "grid-color", "", "Color of main grid lines", "<R,G,B>", "" },
      { "edges", "e", "Show cell edges", "<bool>", "1" },
      { "camera-index", "", "Select the camera to use", "<index>", "" },
      { "trackball", "k", "Enable trackball interaction", "<bool>", "1" },
      { "invert-zoom", "", "Invert zoom direction with right mouse click", "<bool>", "1" },
      { "animation-autoplay", "", "Automatically start animation", "<bool>", "1" },
      { "animation-index", "", "Select animation to show", "<index>", "" },
      { "animation-speed-factor", "", "Set animation speed factor", "<factor>", "" },
      { "animation-time", "", "Set animation time to load", "<time>", "" },
      {"animation-frame-rate", "", "Set animation frame rate when playing animation interactively", "<frame rate>", ""},
      {"font-file", "", "Path to a FreeType compatible font file", "<file_path>", ""} } },
  { "Material",
    { {"point-sprites", "o", "Show sphere sprites instead of geometry", "<bool>", "1" },
      {"point-type", "", "Point sprites type when showing point sprites", "<sphere|gaussian>", ""},
      {"point-size", "", "Point size when showing vertices or point sprites", "<size>", ""},
      {"line-width", "", "Line width when showing edges", "<width>", ""},
      {"backface-type", "", "Backface type, can be default (usually visible), visible or hidden", "<default|visible|hidden>", ""},
      {"color", "", "Solid color", "<R,G,B>", ""},
      {"opacity", "", "Opacity", "<opacity>", ""},
      {"roughness", "", "Roughness coefficient (0.0-1.0)", "<roughness>", ""},
      {"metallic", "", "Metallic coefficient (0.0-1.0)", "<metallic>", ""},
      {"hdri-file", "", "Path to an image file that can be used as a light source and skybox", "<file path>", ""},
      {"hdri-ambient", "f", "Enable HDRI ambient lighting", "<bool>", "1"},
      {"hdri-skybox", "j", "Enable HDRI skybox background", "<bool>", "1"},
      {"texture-matcap", "", "Path to a texture file containing a material capture", "<file path>", ""},
      {"texture-base-color", "", "Path to a texture file that sets the color of the object", "<file path>", ""},
      {"texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", "<file path>", ""},
      {"texture-emissive", "", "Path to a texture file that sets the emitted light of the object", "<file path>", ""},
      {"emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", "<R,G,B>", ""},
      {"texture-normal", "", "Path to a texture file that sets the normal map of the object", "<file path>", ""},
      {"normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", "<normalScale>", ""} } },
  {"Window",
    { {"bg-color", "", "Background color", "<R,G,B>", ""},
      {"resolution", "", "Window resolution", "<width,height>", ""},
      {"position", "", "Window position", "<x,y>", ""},
      {"fps", "z", "Display frame per second", "<bool>", "1"},
      {"filename", "n", "Display filename", "<bool>", "1"},
      {"metadata", "m", "Display file metadata", "<bool>", "1"},
      {"blur-background", "u", "Blur background", "<bool>", "1" },
      {"blur-coc", "", "Blur circle of confusion radius", "<value>", ""},
      {"light-intensity", "", "Light intensity", "<value>", ""} } },
  {"Scientific visualization",
    { {"scalar-coloring", "s", "Color by a scalar array", "<bool>", "1" },
      {"coloring-array", "", "Name of the array to color with", "<array_name>", "" },
      {"comp", "y", "Component from the array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", "<comp_index>", "-2"},
      {"cells", "c", "Use an array from the cells", "<bool>", "1"},
      {"range", "", "Custom range for the coloring by array", "<min,max>", ""},
      {"bar", "b", "Show scalar bar", "<bool>", "1" },
      {"colormap-file", "", "Specify a colormap image", "<filePath/filename/fileStem>", ""},
      {"colormap", "", "Specify a custom colormap (ignored if \"colormap-file\" is specified)", "<color_list>", ""},
      {"volume", "v", "Show volume if the file is compatible", "<bool>", "1"},
      {"inverse", "i", "Inverse opacity function for volume rendering", "<bool>", "1"} } },
  {"Camera",
    { {"camera-position", "", "Camera position (overrides camera direction and camera zoom factor if any)", "<X,Y,Z>", ""},
      {"camera-focal-point", "", "Camera focal point", "<X,Y,Z>", ""},
      {"camera-view-up", "", "Camera view up", "<X,Y,Z>", ""},
      {"camera-view-angle", "", "Camera view angle (non-zero, in degrees)", "<angle>", ""},
      {"camera-direction", "", "Camera direction", "<X,Y,Z>", ""},
      {"camera-zoom-factor", "", "Camera zoom factor (non-zero)", "<factor>", ""},
      {"camera-azimuth-angle", "", "Camera azimuth angle (in degrees), performed after other camera options", "<angle>", ""},
      {"camera-elevation-angle", "", "Camera elevation angle (in degrees), performed after other camera options", "<angle>", ""},
      {"camera-orthographic", "", "Use an orthographic camera", "<bool>", "1"} } },

#if F3D_MODULE_RAYTRACING
  {"Raytracing",
    { {"raytracing", "r", "Enable raytracing", "<bool>", "1"},
      {"samples", "", "Number of samples per pixel", "<samples>", ""},
      {"denoise", "d", "Denoise the image", "<bool>", "1"} } },
#endif
  {"PostFX (OpenGL)",
    { {"translucency-support", "p", "Enable translucency support, implemented using depth peeling", "<bool>", "1"},
      {"ambient-occlusion", "q", "Enable ambient occlusion providing approximate shadows for better depth perception, implemented using SSAO", "<bool>", "1"},
      {"anti-aliasing", "a", "Enable anti-aliasing, implemented using FXAA", "<bool>", "1"},
      {"tone-mapping", "t", "Enable Tone Mapping, providing balanced coloring", "<bool>", "1"},
      {"final-shader", "", "Execute the final shader at the end of the rendering pipeline", "<GLSL code>", ""} } },
  {"Testing",
    { {"ref", "", "Reference", "<png file>", ""},
      {"ref-threshold", "", "Testing threshold", "<threshold>", ""},
      {"interaction-test-record", "", "Path to an interaction log file to record interactions events to", "<file_path>", ""},
      {"interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", "<file_path>", ""} } }
}};

/**
 * True boolean options need to be filtered out in ParseCLIOptions
 * This is the easiest, compile time way to do it
 */
constexpr std::array<std::string_view, 4> CLIBooleans = {"version", "help", "readers-list", "scan-plugins"};

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
void PrintHelpPair(
  std::string_view key, std::string_view help, int keyWidth = 10, int helpWidth = 70)
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
void PrintHelp(const std::string& execName, const cxxopts::Options& cxxOptions)
{
  const std::array<std::pair<std::string, std::string>, 4> examples = {{
    { execName + " file.vtu -xtgans",
      "View a unstructured mesh in a typical nice looking sciviz style" },
    { execName + " file.glb -tuqap --hdri-file=file.hdr --hdri-ambient --hdri-skybox",
      "View a gltf file in a realistic environment" },
    { execName + " file.ply -so --point-size=0 --comp=-2",
      "View a point cloud file with direct scalars rendering" },
    { execName + " folder", "View all files in folder" },
  }};

  f3d::log::setUseColoring(false);
  std::vector<std::string> orderedCLIGroupNames;
  orderedCLIGroupNames.reserve(::CLIOptions.size());
  for (const ::CLIGroup& optionGroup : ::CLIOptions)
  {
    orderedCLIGroupNames.emplace_back(optionGroup.GroupName);
  }
  f3d::log::info(cxxOptions.help(orderedCLIGroupNames));
    f3d::log::info("Keys:");
  for (const auto& [key, desc] : f3d::interactor::getDefaultInteractionsInfo())
  {
    ::PrintHelpPair(key, desc);
  }

  f3d::log::info("\nExamples:");
  for (const auto& [cmd, desc] : examples)
  {
    ::PrintHelpPair(cmd, desc, 50);
  }
  f3d::log::info("\nReport bugs to https://github.com/f3d-app/f3d/issues");
  f3d::log::setUseColoring(true);
  f3d::log::waitForUser();
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

  auto plugins = f3d::engine::getPluginsList(appPath.string());

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
  f3d::log::waitForUser();
}

//----------------------------------------------------------------------------
void PrintReadersList()
{
  size_t nameColSize = 0;
  size_t extsColSize = 0;
  size_t mimeColSize = 0;
  size_t descColSize = 0;
  size_t plugColSize = 0;

  std::vector<f3d::engine::readerInformation> readersInfo = f3d::engine::getReadersInfo();
  if (readersInfo.empty())
  {
    f3d::log::warn("No registered reader found!");
    return;
  }
  // Compute the size of the 5 columns
  for (const auto& reader : readersInfo)
  {
    // There is at least one MIME type for each extension
    assert(reader.Extensions.size() >= reader.MimeTypes.size());

    nameColSize = std::max(nameColSize, reader.Name.length());
    descColSize = std::max(descColSize, reader.Description.length());
    plugColSize = std::max(plugColSize, reader.PluginName.length());

    for (const auto& ext : reader.Extensions)
    {
      extsColSize = std::max(extsColSize, ext.length());
    }
    for (const auto& mime : reader.MimeTypes)
    {
      mimeColSize = std::max(mimeColSize, mime.length());
    }
  }
  const size_t colGap = 4;
  nameColSize += colGap;
  extsColSize += colGap;
  mimeColSize += colGap;
  descColSize += colGap;
  plugColSize += colGap;

  std::string separator =
    std::string(nameColSize + extsColSize + descColSize + mimeColSize + plugColSize - colGap, '-');

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(plugColSize) << "Plugin"
             << std::setw(descColSize) << "Description" << std::setw(extsColSize) << "Exts"
             << std::setw(mimeColSize - colGap) << "Mime-types";
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
      readerLine << std::setw(extsColSize)
                 << (i < reader.Extensions.size() ? reader.Extensions[i] : "");
      readerLine << std::setw(mimeColSize - colGap)
                 << (i < reader.MimeTypes.size() ? reader.MimeTypes[i] : "");

      f3d::log::info(readerLine.str());
    }
  }
  f3d::log::waitForUser();
}
}

//----------------------------------------------------------------------------
std::pair<std::string, int> F3DOptionsTools::GetClosestOption(const std::string& option, bool checkLib)
{
  std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };
  auto checkDistance = [](const std::string& key, const std::string& name, std::pair<std::string, int>& ref) {
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

  // Check cli names for libf3d options
  for (const auto& [key, value] : F3DOptionsTools::LibOptionsNames)
  {
    checkDistance(std::string(key), option, ret);
  }

  // Check libf3d option names
  if (checkLib)
  {
    f3d::options opt;
    for (const std::string& key : opt.getNames())
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
  try
  {
    cxxopts::Options cxxOptions(execName, F3D::AppTitle);
    cxxOptions.custom_help("[OPTIONS...]");

    for (const ::CLIGroup& optionGroup : ::CLIOptions)
    {
      auto group = cxxOptions.add_options(std::string(optionGroup.GroupName));

      // Positional option, `--input` require a custom implementation
      if (std::string(optionGroup.GroupName) == "Applicative")
      {
        group("input", "Input files", cxxoptsInputPositionals, "<files>");
      }

      // Add each option to cxxopts
      for (const ::CLIOption& cliOption : optionGroup.Options)
      {
        if (cliOption.ValueHelper.empty())
        {
          // No ValueHelper means its a true boolean option like `--help` or `--version`
          group(::CollapseName(cliOption.LongName, cliOption.ShortName), std::string(cliOption.HelpText));
        }
        else
        {
          // Add the default value to the help text if any
          std::string defaultValue;
          std::string helpText(cliOption.HelpText);

          // Recover default value from app options
          auto appIter = F3DOptionsTools::DefaultAppOptions.find(std::string(cliOption.LongName));
          if (appIter != F3DOptionsTools::DefaultAppOptions.end())
          {
            defaultValue = appIter->second;
          }
          else
          {
            // Recover default value from lib options
            auto libIter = F3DOptionsTools::LibOptionsNames.find(cliOption.LongName);
            if (libIter != F3DOptionsTools::LibOptionsNames.end())
            {
              f3d::options opt;
              defaultValue = opt.getAsString(std::string(libIter->second));
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
        const std::string closestOption =
          equalPos == std::string::npos ? closestName : closestName + unknownOption.substr(equalPos);

        f3d::log::error("Did you mean '--", closestOption, "'?");
      }
    }
    if (foundUnknownOption)
    {
      f3d::log::waitForUser();
      throw F3DExFailure("unknown options");
    }

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
    if (result.count("scan-plugins") > 0)
    {
      ::PrintPluginsScan();
      throw F3DExNoProcess("scan plugins requested");
    }
    if (result.count("readers-list") > 0)
    {
      // `--readers-list` needs plugin to be loaded to be useful
      // Load them manually
      std::vector<std::string> plugins;
      if (result.count("load-plugins") > 0)
      {
        plugins = f3d::options::parse<std::vector<std::string>>(result["load-plugins"].as<std::string>());
      }
      F3DPluginsTools::LoadPlugins(plugins);
      ::PrintReadersList();
      throw F3DExNoProcess("reader list requested");
    }

    // Add each CLI options into a vector of string/string and return it
    F3DOptionsTools::OptionsDict cliOptionsDict;
    for (const auto& res : result)
    {
      // Discard boolean option like `--version` or `--help`
      if (std::find(::CLIBooleans.begin(), ::CLIBooleans.end(), res.key()) == ::CLIBooleans.end())
      {
        cliOptionsDict.emplace(res.key(), res.value());
      }
    }
    return cliOptionsDict;
  }
  catch (const cxxopts::exceptions::exception& ex)
  {
    f3d::log::error("Error parsing command line arguments: ", ex.what());
    throw F3DExFailure("Could not parse command line arguments");
  }
}
