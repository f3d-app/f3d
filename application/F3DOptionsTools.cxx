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
#include <numeric>
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
      { "list-readers", "", "Print the list of readers", "", "" },
      { "force-reader", "", "Force a specific reader to be used, disregarding the file extension", "<reader>", "1"},
      { "list-bindings", "", "Print the list of interaction bindings and exits, ignored with `--no-render`, only considers the first file group.", "<bool>", "1" },
      { "config", "", "Specify the configuration file to use. absolute/relative path or filename/filestem to search in configuration file locations", "<filePath/filename/fileStem>", "" },
      { "no-config", "", "Do not read the configuration file", "<bool>", "1" },
      { "no-render", "", "Do not render anything and quit right after loading the first file, use with --verbose to recover information about a file.", "<bool>", "1" },
      { "rendering-backend", "", "Backend to use when rendering (auto|glx|wgl|egl|osmesa)", "<string>", "" },
      { "list-rendering-backends", "", "Print the list of rendering backends available on this system", "", "" },
      { "max-size", "", "Maximum size in Mib of a file to load, leave empty for unlimited", "<size in Mib>", "" },
#if F3D_MODULE_DMON
      { "watch", "", "Watch current file and automatically reload it whenever it is modified on disk", "<bool>", "1" },
#endif
      { "frame-rate", "", "Frame rate used to refresh animation and other repeated tasks (watch, UI). Does not impact rendering frame rate.", "<fps>", "" },
      { "load-plugins", "", "List of plugins to load separated with a comma", "<paths or names>", "" },
      { "scan-plugins", "", "Scan standard directories for plugins and display available plugins (result can be incomplete)", "", "" },
      { "screenshot-filename", "", "Screenshot filename", "<filename>", "" } } },
  { "General",
    { { "verbose", "", "Set verbose level, providing more information about the loaded data in the console output", "{debug, info, warning, error, quiet}", "debug" },
      { "loading-progress", "", "Show loading progress bar", "<bool>", "1" },
      { "animation-progress", "", "Show animation progress bar", "<bool>", "1" },
      { "multi-file-mode", "", R"(Choose the behavior when opening multiple files. "single" will show one file at a time, "all" will show all files in a single scene, "dir" will show files from the same directory in the same scene.)", "<single|all|dir>", "" },
      { "multi-file-regex", "", R"(Regular expression pattern to group files)", "<regex>", "" },
      { "recursive-dir-add", "", "Add directories recursively", "<bool>", "1" },
      { "remove-empty-file-groups", "", "Remove file groups that results into an empty scene", "<bool>", "1" },
      { "up", "", "Up direction", "<direction>", "" },
      { "axis", "x", "Show axes", "<bool>", "1" }, { "grid", "g", "Show grid", "<bool>", "1" },
      { "grid-absolute", "", "Position grid at the absolute origin instead of below the model", "<bool>", "1" },
      { "grid-unit", "", "Size of grid unit square, automatically computed by default", "<value>", "" },
      { "grid-subdivisions", "", "Number of grid subdivisions", "<value>", "" },
      { "grid-color", "", "Color of main grid lines", "<color>", "" },
      { "axes-grid", "", "Enable grid axis", "<bool>", "1" },
      { "edges", "e", "Show cell edges", "<bool>", "1" },
      { "armature", "", "Enable armature visualization", "<bool>", "1" },
      { "camera-index", "", "Select the camera to use", "<index>", "" },
      { "interaction-trackball", "k", "Enable trackball interaction", "<bool>", "1" },
      { "invert-zoom", "", "Invert zoom direction with right mouse click", "<bool>", "1" },
      { "animation-autoplay", "", "Automatically start animation", "<bool>", "1" },
      { "animation-index", "", "Select animation to show (deprecated)", "<index>", "" },
      { "animation-indices", "", "Select animations to show", "<index,index,index>", "" },
      { "animation-speed-factor", "", "Set animation speed factor", "<ratio>", "" },
      { "animation-time", "", "Set animation time to load", "<time>", "" },
      { "font-file", "", "Path to a FreeType compatible font file", "<file_path>", ""},
      { "font-scale", "", "Scale fonts", "<ratio>", ""},
      { "command-script", "", "Path to a script file containing commands to execute", "<file_path>", "" } } },
  { "Material",
    { {"point-sprites", "o", "Show sphere sprites instead of surfaces", "<bool>", "1" },
      {"point-sprites-type", "", "Point sprites type", "<sphere|gaussian>", ""},
      {"point-sprites-size", "", "Point sprites size", "<size>", ""},
      {"point-size", "", "Point size when showing vertices, model specified by default", "<size>", ""},
      {"line-width", "", "Line width when showing edges, model specified by default", "<width>", ""},
      {"backface-type", "", "Backface type, can be visible or hidden, model specified by default", "<visible|hidden>", ""},
      {"color", "", "Solid color", "<color>", ""},
      {"opacity", "", "Opacity", "<opacity>", ""},
      {"roughness", "", "Roughness coefficient (0.0-1.0)", "<roughness>", ""},
      {"metallic", "", "Metallic coefficient (0.0-1.0)", "<metallic>", ""},
      {"base-ior", "", "Index Of Refraction of the base layer (1.0-2.5)", "<base-ior>", ""},
      {"hdri-file", "", "Path to an image file that can be used as a light source and skybox", "<file path>", ""},
      {"hdri-ambient", "f", "Enable HDRI ambient lighting", "<bool>", "1"},
      {"hdri-skybox", "j", "Enable HDRI skybox background", "<bool>", "1"},
      {"texture-matcap", "", "Path to a texture file containing a material capture", "<file path>", ""},
      {"texture-base-color", "", "Path to a texture file that sets the color of the object", "<file path>", ""},
      {"texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", "<file path>", ""},
      {"texture-emissive", "", "Path to a texture file that sets the emitted light of the object", "<file path>", ""},
      {"textures-transform", "", "3x3 matrix to transform textures", "<transform2d>", ""},
      {"emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", "<color>", ""},
      {"texture-normal", "", "Path to a texture file that sets the normal map of the object", "<file path>", ""},
      {"normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", "<normalScale>", ""} } },
  {"Window",
    { {"background-color", "", "Background color", "<color>", ""},
      {"resolution", "", "Window resolution", "<width,height>", ""},
      {"position", "", "Window position", "<x,y>", ""},
      {"fps", "z", "Display rendering frame per second", "<bool>", "1"},
      {"filename", "n", "Display filename", "<bool>", "1"},
      {"metadata", "m", "Display file metadata", "<bool>", "1"},
      {"blur-background", "u", "Blur background", "<bool>", "1" },
      {"blur-coc", "", "Blur circle of confusion radius", "<value>", ""},
      {"light-intensity", "", "Light intensity", "<value>", ""} } },
  {"Scientific visualization",
    { {"scalar-coloring", "s", "Color by a scalar array", "<bool>", "1" },
      {"coloring-array", "", "Name of the array to color with", "<array_name>", "" },
      {"coloring-component", "y", "Component from the array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", "<comp_index>", "-2"},
      {"coloring-by-cells", "c", "Use an array from the cells", "<bool>", "1"},
      {"coloring-range", "", "Custom range for the coloring by array, automatically computed by default", "<min,max>", ""},
      {"coloring-scalar-bar", "b", "Show scalar bar", "<bool>", "1" },
      {"colormap-file", "", "Specify a colormap image", "<filePath/filename/fileStem>", ""},
      {"colormap-discretization", "", "Specify number of colors in colormap", "<int>", ""},
      {"colormap", "", "Specify a custom colormap (ignored if \"colormap-file\" is specified)", "<color_list>", ""},
      {"volume", "v", "Show volume if the file is compatible", "<bool>", "1"},
      {"volume-inverse", "i", "Inverse opacity function for volume rendering", "<bool>", "1"} } },
  {"Camera",
    { {"camera-position", "", "Camera position (overrides camera direction and camera zoom factor if any)", "<X,Y,Z>", ""},
      {"camera-focal-point", "", "Camera focal point", "<X,Y,Z>", ""},
      {"camera-view-up", "", "Camera view up", "<direction>", ""},
      {"camera-view-angle", "", "Camera view angle (non-zero, in degrees)", "<angle>", ""},
      {"camera-direction", "", "Camera direction", "<direction>", ""},
      {"camera-zoom-factor", "", "Camera zoom factor (non-zero)", "<factor>", ""},
      {"camera-azimuth-angle", "", "Camera azimuth angle (in degrees), performed after other camera options", "<angle>", ""},
      {"camera-elevation-angle", "", "Camera elevation angle (in degrees), performed after other camera options", "<angle>", ""},
      {"camera-orthographic", "", "Use an orthographic camera", "<bool>", "1"} } },

#if F3D_MODULE_RAYTRACING
  {"Raytracing",
    { {"raytracing", "r", "Enable raytracing", "<bool>", "1"},
      {"raytracing-samples", "", "Number of samples per pixel", "<samples>", ""},
      {"raytracing-denoise", "d", "Denoise the image", "<bool>", "1"} } },
#endif
  {"PostFX (OpenGL)",
    { {"translucency-support", "p", "Enable translucency support, implemented using depth peeling", "<bool>", "1"},
      {"ambient-occlusion", "q", "Enable ambient occlusion providing approximate shadows for better depth perception, implemented using SSAO", "<bool>", "1"},
      {"anti-aliasing", "a", "Enable anti-aliasing", "<bool>", "1"},
      {"anti-aliasing-mode", "", R"(Select anti-aliasing method ("fxaa" or "ssaa"))", "<string>", "fxaa"},
      {"tone-mapping", "t", "Enable Tone Mapping, providing balanced coloring", "<bool>", "1"},
      {"final-shader", "", "Execute the final shader at the end of the rendering pipeline", "<GLSL code>", ""} } },
  {"Testing",
    { {"reference", "", "Reference", "<png file>", ""},
      {"reference-threshold", "", "Testing threshold", "<threshold>", ""},
      {"interaction-test-record", "", "Path to an interaction log file to record interactions events to", "<file_path>", ""},
      {"interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", "<file_path>", ""} } }
}};

/**
 * True boolean options need to be filtered out in ParseCLIOptions
 * Also filter out special options like `define` and `reset`
 * This is the easiest, compile time way to do it
 */
constexpr std::array CLIBooleans = {"version", "help", "list-readers", "scan-plugins", "list-rendering-backends", "define", "reset"};

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
  const std::array<std::pair<std::string, std::string>, 4> examples = {{
    { execName + " file.vtu -xtgans",
      "View a unstructured mesh in a typical nice looking sciviz style" },
    { execName + " file.glb -tuqap --hdri-file=file.hdr --hdri-ambient --hdri-skybox",
      "View a gltf file in a realistic environment" },
    { execName + " file.ply -so --point-size=0 --coloring-component=-2",
      "View a point cloud file with direct scalars rendering" },
    { execName + " folder", "View all files in folder" },
  }};

  f3d::log::setUseColoring(false);
  std::vector<std::string> orderedCLIGroupNames(CLIOptions.size());
  std::transform(CLIOptions.cbegin(), CLIOptions.cend(), orderedCLIGroupNames.begin(),
    [](const ::CLIGroup& cliGroup) { return cliGroup.GroupName; });
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
}
}

//----------------------------------------------------------------------------
std::pair<std::string, int> F3DOptionsTools::GetClosestOption(const std::string& option, bool checkLibAndReaders)
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

    for (const ::CLIGroup& optionGroup : ::CLIOptions)
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
              std::string name = std::string(libIter->second);
              // let default value empty for unset options
              defaultValue = opt.hasValue(name) ? opt.getAsString(name) : "";
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
        plugins = f3d::options::parse<std::vector<std::string>>(result["load-plugins"].as<std::string>());
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
        const std::string closestOption =
          equalPos == std::string::npos ? closestName : closestName + unknownOption.substr(equalPos);

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
