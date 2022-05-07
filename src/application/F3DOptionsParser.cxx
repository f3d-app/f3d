#include "F3DOptionsParser.h"

#include "cxxopts.hpp"
#include "json.hpp"

#include "F3DException.h"
#include "f3d_config.h"
#include "f3d_engine.h"
#include "f3d_log.h"
#include "f3d_options.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>

#include "F3DExecPath.h"
//----------------------------------------------------------------------------
class ConfigurationOptions
{
public:
  ConfigurationOptions(int argc, char** argv)
    : Argc(argc)
    , Argv(argv)
  {
  }

  void GetOptionsFromArgs(
    F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& inputs);
  bool InitializeDictionaryFromConfigFile(const std::string& userConfigFile);

  void SetFilePathForConfigBlock(const std::string& filePath)
  {
    this->FilePathForConfigBlock = filePath;
  }

protected:
  bool GetOptionConfig(const std::string& option, std::string& configValue) const
  {
    bool ret = false;
    for (auto const& it : this->ConfigDic)
    {
      std::regex re(it.first);
      std::smatch matches;
      if (std::regex_match(this->FilePathForConfigBlock, matches, re))
      {
        auto localIt = it.second.find(option);
        if (localIt != it.second.end())
        {
          configValue = localIt->second;
          ret = true;
        }
      }
    }
    return ret;
  }

  template<class T>
  static std::string ToString(T currValue)
  {
    std::stringstream ss;
    ss << currValue;
    return ss.str();
  }

  static std::string ToString(bool currValue) { return currValue ? "true" : "false"; }

  template<class T>
  static std::string ToString(const std::vector<T>& currValue)
  {
    std::stringstream ss;
    for (size_t i = 0; i < currValue.size(); i++)
    {
      ss << currValue[i];
      if (i != currValue.size() - 1)
      {
        ss << ",";
      }
    }
    return ss.str();
  }

  std::string CollapseName(const std::string& longName, const std::string& shortName) const
  {
    std::stringstream ss;
    if (shortName != "")
    {
      ss << shortName << ",";
    }
    ss << longName;
    return ss.str();
  }

  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc) const
  {
    group(this->CollapseName(longName, shortName), doc);
  }

  template<class T>
  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, bool hasDefault = true,
    bool mayHaveConfig = true, const std::string& argHelp = "") const
  {
    auto val = cxxopts::value<T>(var);
    std::string defaultVal;
    if (hasDefault)
    {
      defaultVal = ConfigurationOptions::ToString(var);
    }

    if (mayHaveConfig)
    {
      hasDefault |= this->GetOptionConfig(longName, defaultVal);
    }

    if (hasDefault)
    {
      val = val->default_value(defaultVal);
    }
    var = {};
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  template<class T>
  void DeclareOptionWithImplicitValue(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, const std::string& implicitValue,
    bool hasDefault = true, bool mayHaveConfig = true, const std::string& argHelp = "") const
  {
    auto val = cxxopts::value<T>(var)->implicit_value(implicitValue);
    std::string defaultVal;
    if (hasDefault)
    {
      defaultVal = ConfigurationOptions::ToString(var);
    }

    if (mayHaveConfig)
    {
      hasDefault |= this->GetOptionConfig(longName, defaultVal);
    }

    if (hasDefault)
    {
      val = val->default_value(defaultVal);
    }

    var = {};
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  std::string GetBinaryConfigFileDirectory();
  std::string GetConfigFilePath();

  static std::string GetSystemConfigFileDirectory();
  static std::string GetUserConfigFileDirectory();

  void PrintHelpPair(
    const std::string& key, const std::string& help, int keyWidth = 10, int helpWidth = 70);
  void PrintHelp(cxxopts::Options& cxxOptions);
  void PrintVersion();
  void PrintReadersList();

private:
  int Argc;
  char** Argv;

  std::string FilePathForConfigBlock;

  using Dictionnary = std::map<std::string, std::map<std::string, std::string> >;
  Dictionnary ConfigDic;
};

//----------------------------------------------------------------------------
void ConfigurationOptions::GetOptionsFromArgs(
  F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& inputs)
{
  try
  {
    // TODO From where ?
    cxxopts::Options cxxOptions("F3D", "F3D");
    cxxOptions.positional_help("file1 file2 ...");

    // clang-format off
    auto grp0 = cxxOptions.add_options("Applicative");
    this->DeclareOption(grp0, "input", "", "Input file", inputs, false, false, "<files>");
    this->DeclareOption(grp0, "output", "", "Render to file", appOptions.Output, false, false,"<png file>");
    this->DeclareOption(grp0, "no-background", "", "No background when render to file", appOptions.NoBackground);
    this->DeclareOption(grp0, "help", "h", "Print help");
    this->DeclareOption(grp0, "version", "", "Print version details");
    this->DeclareOption(grp0, "readers-list", "", "Print the list of file types");
    this->DeclareOption(grp0, "config", "", "Read a provided configuration file instead of default one", appOptions.UserConfigFile, false, false, "<file path>");
    this->DeclareOption(grp0, "dry-run", "", "Do not read the configuration file", appOptions.DryRun, true, false);
    this->DeclareOption(grp0, "no-render", "", "Verbose mode without any rendering, only for the first file", appOptions.NoRender, true, false);

    auto grp1 = cxxOptions.add_options("General");
    this->DeclareOption(grp1, "verbose", "", "Enable verbose mode, providing more information about the loaded data in the console output", options.getAsBoolRef("verbose"));
    this->DeclareOption(grp1, "quiet", "", "Enable quiet mode, which superseed any verbose options and prevent any console output to be generated at all", options.getAsBoolRef("quiet"));
    this->DeclareOption(grp1, "progress", "", "Show progress bar", options.getAsBoolRef("progress"));
    this->DeclareOption(grp1, "geometry-only", "", "Do not read materials, cameras and lights from file", options.getAsBoolRef("geometry-only"));
    this->DeclareOption(grp1, "up", "", "Up direction", options.getAsStringRef("up"), true, "[-X|+X|-Y|+Y|-Z|+Z]");
    this->DeclareOption(grp1, "axis", "x", "Show axes", options.getAsBoolRef("axis"));
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.getAsBoolRef("grid"));
    this->DeclareOption(grp1, "edges", "e", "Show cell edges", options.getAsBoolRef("edges"));
    this->DeclareOption(grp1, "camera-index", "", "Select the camera to use", options.getAsIntRef("camera-index"), true, true, "<index>");
    this->DeclareOption(grp1, "trackball", "k", "Enable trackball interaction", options.getAsBoolRef("trackball"));
    this->DeclareOption(grp1, "animation-index", "", "Select animation to show", options.getAsIntRef("animation-index"), true, true, "<index>");
    this->DeclareOption(grp1, "font-file", "", "Path to a FreeType compatible font file", options.getAsStringRef("font-file"), false, false, "<file_path>");

    auto grp2 = cxxOptions.add_options("Material");
    this->DeclareOption(grp2, "point-sprites", "o", "Show sphere sprites instead of geometry", options.getAsBoolRef("point-sprites"));
    this->DeclareOption(grp2, "point-size", "", "Point size when showing vertices or point sprites", options.getAsDoubleRef("point-size"), true, true, "<size>");
    this->DeclareOption(grp2, "line-width", "", "Line width when showing edges", options.getAsDoubleRef("line-width"), true, true, "<width>");
    this->DeclareOption(grp2, "color", "", "Solid color", options.getAsDoubleVectorRef("color"), true, true, "<R,G,B>");
    this->DeclareOption(grp2, "opacity", "", "Opacity", options.getAsDoubleRef("opacity"), true, true, "<opacity>");
    this->DeclareOption(grp2, "roughness", "", "Roughness coefficient (0.0-1.0)", options.getAsDoubleRef("roughness"), true, true, "<roughness>");
    this->DeclareOption(grp2, "metallic", "", "Metallic coefficient (0.0-1.0)", options.getAsDoubleRef("metallic"), true, true, "<metallic>");
    this->DeclareOption(grp2, "hdri", "", "Path to an image file that will be used as a light source", options.getAsStringRef("hdri"), false, true, "<file path>");
    this->DeclareOption(grp2, "texture-base-color", "", "Path to a texture file that sets the color of the object", options.getAsStringRef("texture-base-color"), false, true, "<file path>");
    this->DeclareOption(grp2, "texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", options.getAsStringRef("texture-material"), false, true, "<file path>");
    this->DeclareOption(grp2, "texture-emissive", "", "Path to a texture file that sets the emitted light of the object", options.getAsStringRef("texture-emissive"), false, true, "<file path>");
    this->DeclareOption(grp2, "emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", options.getAsDoubleVectorRef("emissive-factor"), true, true, "<R,G,B>");
    this->DeclareOption(grp2, "texture-normal", "", "Path to a texture file that sets the normal map of the object", options.getAsStringRef("texture-normal"), false, true, "<file path>");
    this->DeclareOption(grp2, "normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", options.getAsDoubleRef("normal-scale"), true, true, "<normalScale>");

    auto grp3 = cxxOptions.add_options("Window");
    this->DeclareOption(grp3, "bg-color", "", "Background color", options.getAsDoubleVectorRef("background-color"), true, true, "<R,G,B>"); // TODO change name ?
    this->DeclareOption(grp3, "resolution", "", "Window resolution", options.getAsIntVectorRef("resolution"), true, true, "<width,height>");
    this->DeclareOption(grp3, "fps", "z", "Display frame per second", options.getAsBoolRef("fps"));
    this->DeclareOption(grp3, "filename", "n", "Display filename", options.getAsBoolRef("filename"));
    this->DeclareOption(grp3, "metadata", "m", "Display file metadata", options.getAsBoolRef("metadata"));
    this->DeclareOption(grp3, "fullscreen", "f", "Full screen", options.getAsBoolRef("fullscreen"));
    this->DeclareOption(grp3, "blur-background", "u", "Blur background", options.getAsBoolRef("blur-background"));

    auto grp4 = cxxOptions.add_options("Scientific visualization");
    this->DeclareOptionWithImplicitValue(grp4, "scalars", "s", "Color by scalars", options.getAsStringRef("scalars"), std::string(""), true, true, "<array_name>");
    this->DeclareOptionWithImplicitValue(grp4, "comp", "y", "Component from the scalar array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", options.getAsIntRef("component"), "-2", true, true, "<comp_index>");
    this->DeclareOption(grp4, "cells", "c", "Use a scalar array from the cells", options.getAsBoolRef("cells"));
    this->DeclareOption(grp4, "range", "", "Custom range for the coloring by array", options.getAsDoubleVectorRef("range"), false, true, "<min,max>");
    this->DeclareOption(grp4, "bar", "b", "Show scalar bar", options.getAsBoolRef("bar"));
    this->DeclareOption(grp4, "colormap", "", "Specify a custom colormap", options.getAsDoubleVectorRef("colormap"), true, "<color_list>");
    this->DeclareOption(grp4, "volume", "v", "Show volume if the file is compatible", options.getAsBoolRef("volume"));
    this->DeclareOption(grp4, "inverse", "i", "Inverse opacity function for volume rendering", options.getAsBoolRef("inverse"));

    auto grpCamera = cxxOptions.add_options("Camera");
    this->DeclareOption(grpCamera, "camera-position", "", "Camera position", options.getAsDoubleVectorRef("camera-position"), false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-focal-point", "", "Camera focal point", options.getAsDoubleVectorRef("camera-focal-point"), false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-up", "", "Camera view up", options.getAsDoubleVectorRef("camera-view-up"), false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-angle", "", "Camera view angle (non-zero, in degrees)", options.getAsDoubleRef("camera-view-angle"), false, true, "<angle>");
    this->DeclareOption(grpCamera, "camera-azimuth-angle", "", "Camera azimuth angle (in degrees)", options.getAsDoubleRef("camera-azimuth-angle"), true, true, "<angle>");
    this->DeclareOption(grpCamera, "camera-elevation-angle", "", "Camera elevation angle (in degrees)", options.getAsDoubleRef("camera-elevation-angle"), true, true, "<angle>");

    // TODO find the right way to deal with modular options
    auto grp5 = cxxOptions.add_options("Raytracing");
    this->DeclareOption(grp5, "raytracing", "r", "Enable raytracing", options.getAsBoolRef("raytracing"));
    this->DeclareOption(grp5, "samples", "", "Number of samples per pixel", options.getAsIntRef("samples"), true, true, "<samples>");
    this->DeclareOption(grp5, "denoise", "d", "Denoise the image", options.getAsBoolRef("denoise"));

    auto grp6 = cxxOptions.add_options("PostFX (OpenGL)");
    this->DeclareOption(grp6, "depth-peeling", "p", "Enable depth peeling", options.getAsBoolRef("depth-peeling"));
    this->DeclareOption(grp6, "ssao", "q", "Enable Screen-Space Ambient Occlusion", options.getAsBoolRef("ssao"));
    this->DeclareOption(grp6, "fxaa", "a", "Enable Fast Approximate Anti-Aliasing", options.getAsBoolRef("fxaa"));
    this->DeclareOption(grp6, "tone-mapping", "t", "Enable Tone Mapping", options.getAsBoolRef("tone-mapping"));
    
    auto grp7 = cxxOptions.add_options("Testing");
    this->DeclareOption(grp7, "ref", "", "Reference", appOptions.Reference, false, false, "<png file>");
    this->DeclareOption(grp7, "ref-threshold", "", "Testing threshold", appOptions.RefThreshold, true, false, "<threshold>");
    this->DeclareOption(grp7, "interaction-test-record", "", "Path to an interaction log file to record interactions events to", appOptions.InteractionTestRecordFile, false, false, "<file_path>");
    this->DeclareOption(grp7, "interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", appOptions.InteractionTestPlayFile, false, false,"<file_path>");
    // clang-format on

    cxxOptions.parse_positional({ "input" });

    auto result = cxxOptions.parse(this->Argc, this->Argv);

    if (result.count("help") > 0)
    {
      this->PrintHelp(cxxOptions);
      throw F3DExNoProcess();
    }

    if (result.count("version") > 0)
    {
      this->PrintVersion();
      throw F3DExNoProcess();
    }

    if (result.count("readers-list") > 0)
    {
      this->PrintReadersList();
      throw F3DExNoProcess();
    }
  }
  catch (const cxxopts::OptionException& e)
  {
    f3d::log::error("Error parsing options: ", e.what());
    throw;
  }
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintHelpPair(
  const std::string& key, const std::string& help, int keyWidth, int helpWidth)
{
  std::stringstream ss;
  ss << "  " << std::left << std::setw(keyWidth) << key << " " << std::setw(helpWidth) << help;
  f3d::log::info(ss.str());
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintHelp(cxxopts::Options& cxxOptions)
{
  // TODO this should rely on f3d::engine/interactor in some capacity
  // clang-format off
  const std::vector<std::pair<std::string, std::string> > keys =
  {
    { "C", "Cycle point/cell data coloring" },
    { "S", "Cycle array to color with" },
    { "Y", "Cycle array component to color with" },
    { "B", "Toggle the scalar bar display" },
    { "V", "Toggle volume rendering" },
    { "I", "Toggle inverse volume opacity" },
    { "O", "Toggle point sprites rendering" },
    { "P", "Toggle depth peeling" },
    { "Q", "Toggle SSAO" },
    { "A", "Toggle FXAA" },
    { "T", "Toggle tone mapping" },
    { "E", "Toggle the edges display" },
    { "X", "Toggle the axes display" },
    { "G", "Toggle the grid display" },
    { "N", "Toggle the filename display" },
    { "M", "Toggle the metadata display" },
    { "Z", "Toggle the FPS counter display" },
    { "R", "Toggle raytracing rendering" },
    { "D", "Toggle denoising when raytracing" },
    { "F", "Toggle full screen" },
    { "U", "Toggle blur background" },
    { "K", "Toggle trackball interaction" },
    { "H", "Toggle cheat sheet display" },
    { "?", "Dump camera state to the terminal" },
    { "Escape", "Quit" },
    { "Enter", "Reset camera to initial parameters" },
    { "Space", "Play animation if any" },
    { "Left", "Previous file" },
    { "Right", "Next file" },
    { "Up", "Reload current file" }
  };

  const std::vector<std::pair<std::string, std::string> > examples =
  {
    { "f3d file.vtu -xtgans", "View a unstructured mesh in a typical nice looking sciviz style" },
    { "f3d file.glb -tuqap --hdri=file.hdr", "View a gltf file in a realistic environment" },
    { "f3d file.ply -so --point-size=0 --comp=-2", "View a point cloud file with direct scalars rendering" },
    { "f3d folder", "View all files in folder" },
  };
  // clang-format on

  f3d::log::setUseColoring(false);
  f3d::log::info(cxxOptions.help());
  f3d::log::info("Keys:");
  for (const auto& key : keys)
  {
    this->PrintHelpPair(key.first, key.second);
  }

  f3d::log::info("\nExamples:");
  for (const auto& example : examples)
  {
    this->PrintHelpPair(example.first, example.second, 50);
  }
  f3d::log::info("\nReport bugs to https://github.com/f3d-app/f3d/issues");
  f3d::log::setUseColoring(true);
  f3d::log::waitForUser();
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintVersion()
{
  f3d::engine::printVersion();
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintReadersList()
{
  f3d::engine::printReadersList();
}

//----------------------------------------------------------------------------
bool ConfigurationOptions::InitializeDictionaryFromConfigFile(const std::string& userConfigFile)
{
  this->ConfigDic.clear();

  std::string configFilePath;
  if (!userConfigFile.empty())
  {
    configFilePath = userConfigFile;
  }
  else
  {
    configFilePath = this->GetConfigFilePath();
  }
  if (configFilePath.empty())
  {
    return false;
  }

  // Recover an absolute canonical path to config file
  try
  {
    configFilePath = std::filesystem::canonical(std::filesystem::path(configFilePath)).string();
  }
  catch (const std::filesystem::filesystem_error&)
  {
    f3d::log::error("Configuration file does not exists: ", configFilePath);
    return false;
  }

  // Read config file
  std::ifstream file;
  file.open(configFilePath.c_str());

  if (!file.is_open())
  {
    f3d::log::error("Unable to open the configuration file: ", configFilePath);
    return false;
  }

  nlohmann::json j;

  try
  {
    file >> j;
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("Unable to parse the configuration file ", configFilePath);
    f3d::log::error(ex.what());
    return false;
  }

  for (const auto& regexpConfig : j.items())
  {
    std::map<std::string, std::string> localDic;
    for (const auto& prop : regexpConfig.value().items())
    {
      if (prop.value().is_number() || prop.value().is_boolean())
      {
        localDic[prop.key()] = ToString(prop.value());
      }
      else if (prop.value().is_string())
      {
        localDic[prop.key()] = prop.value().get<std::string>();
      }
      else
      {
        f3d::log::error(prop.key(), " must be a string, a boolean or a number");
        return false;
      }
    }
    this->ConfigDic[regexpConfig.key()] = localDic;
  }

  return true;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetUserConfigFileDirectory()
{
  std::string applicationName = "f3d";
#if defined(_WIN32)
  const char* appData = std::getenv("APPDATA");
  if (!appData)
  {
    return std::string();
  }
  std::string separator("\\");
  std::string directoryPath(appData);
  if (directoryPath[directoryPath.size() - 1] != separator[0])
  {
    directoryPath.append(separator);
  }
  directoryPath += applicationName + separator;
#else
  std::string directoryPath;
  std::string separator("/");

  // Implementing XDG specifications
  const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
  if (xdgConfigHome && strlen(xdgConfigHome) > 0)
  {
    directoryPath = xdgConfigHome;
    if (directoryPath[directoryPath.size() - 1] != separator[0])
    {
      directoryPath += separator;
    }
  }
  else
  {
    const char* home = std::getenv("HOME");
    if (!home)
    {
      return std::string();
    }
    directoryPath = home;
    if (directoryPath[directoryPath.size() - 1] != separator[0])
    {
      directoryPath += separator;
    }
    directoryPath += ".config/";
  }
  directoryPath += applicationName + separator;
#endif
  return directoryPath;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetSystemConfigFileDirectory()
{
  std::string directoryPath = "";
// No support implemented for system wide config file on Windows yet
#ifndef _WIN32
#ifdef __APPLE__
  // Implementing simple /usr/local/etc/ system wide config
  directoryPath = "/usr/local/etc/f3d/";
#else
  // Implementing simple /etc/ system wide config
  directoryPath = "/etc/f3d/";
#endif
#endif
  return directoryPath;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetBinaryConfigFileDirectory()
{
  std::string execPath;
  std::filesystem::path dirPath;
  std::string errorMsg, programFilePath;
  try
  {
    dirPath = std::filesystem::canonical(std::filesystem::path(this->Argv[0]))
      .parent_path()
      .parent_path()
      .string();

#if F3D_MACOS_BUNDLE
    for (auto const& dirEntry : std::filesystem::directory_iterator{ dirPath })
    {
      if (dirEntry.path().filename() == "Resources")
      {
        dirPath = dirEntry.path();
      }
    }
#endif
  }
  catch (const std::filesystem::filesystem_error&)
  {
    f3d::log::error("Cannot recover binary config file directory: ", dirPath.string());
    return std::string();
  }

  // Add last separator that may be missing
  dirPath += "/";
  return dirPath.string();
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetConfigFilePath()
{
  std::string fileName = "config.json";
  std::filesystem::path filePath;
  try
  {
    filePath = std::filesystem::path(ConfigurationOptions::GetUserConfigFileDirectory() + fileName);
    if (!std::filesystem::exists(filePath))
    {
      filePath = std::filesystem::path(this->GetBinaryConfigFileDirectory() + fileName);
      if (!std::filesystem::exists(filePath))
      {
        filePath =
          std::filesystem::path(ConfigurationOptions::GetSystemConfigFileDirectory() + fileName);
        if (!std::filesystem::exists(filePath))
        {
          return std::string();
        }
      }
    }
  }
  catch (const std::filesystem::filesystem_error&)
  {
    f3d::log::error("Error recovering config file path: ", filePath.string());
    return std::string();
  }

  return filePath.string();
}

//----------------------------------------------------------------------------
F3DOptionsParser::F3DOptionsParser() = default;

//----------------------------------------------------------------------------
F3DOptionsParser::~F3DOptionsParser() = default;

//----------------------------------------------------------------------------
void F3DOptionsParser::Initialize(int argc, char** argv)
{
  this->ConfigOptions = std::unique_ptr<ConfigurationOptions>(new ConfigurationOptions(argc, argv));
}

//----------------------------------------------------------------------------
void F3DOptionsParser::InitializeDictionaryFromConfigFile(const std::string& userConfigFile)
{
  this->ConfigOptions->InitializeDictionaryFromConfigFile(userConfigFile);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::GetOptionsFromConfigFile(const std::string& filePath, f3d::options& options)
{
  this->ConfigOptions->SetFilePathForConfigBlock(filePath);
  std::vector<std::string> dummyFiles;
  F3DAppOptions dummyAppOptions;
  return this->ConfigOptions->GetOptionsFromArgs(dummyAppOptions, options, dummyFiles);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::GetOptionsFromCommandLine(
  F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& files)
{
  this->ConfigOptions->SetFilePathForConfigBlock("");
  return this->ConfigOptions->GetOptionsFromArgs(appOptions, options, files);
}

// TODO Remove
#if 0
//----------------------------------------------------------------------------
void F3DOptionsParser::ConvertToNewAPI(const F3DOptions& oldOptions, f3d::options* newOptions)
{
  // TODO rework option names
  // TODO remove some options that do not exist in libf3d (like no-render)

  // Application
  // newOptions->set("config", oldOptions.UserConfigFile);
  // newOptions->set("dry-run", oldOptions.DryRun);
  // newOptions->set("no-render", oldOptions.NoRender);
  //  newOptions->set("interaction-test-play", oldOptions.InteractionTestPlayFile);
  //  newOptions->set("interaction-test-record", oldOptions.InteractionTestRecordFile);
  //  newOptions->set("output", oldOptions.Output);
  //  newOptions->set("ref-threshold", oldOptions.RefThreshold);
  //  newOptions->set("reference", oldOptions.Reference);
  //  newOptions->set("no-background", oldOptions.NoBackground);

  // General
  newOptions->set("quiet", oldOptions.Quiet);
  newOptions->set("verbose", oldOptions.Verbose);

  // Loader/Loading
  newOptions->set("animation-index", oldOptions.AnimationIndex);
  newOptions->set("geometry-only", oldOptions.GeometryOnly);
  newOptions->set("progress", oldOptions.Progress);
  newOptions->set("camera-index", oldOptions.CameraIndex);
  newOptions->set("color", oldOptions.SolidColor);
  newOptions->set("emissive-factor", oldOptions.EmissiveFactor);
  newOptions->set("line-width", oldOptions.LineWidth);
  newOptions->set("metallic", oldOptions.Metallic);
  newOptions->set("normal-scale", oldOptions.NormalScale);
  newOptions->set("opacity", oldOptions.Opacity);
  newOptions->set("point-size", oldOptions.PointSize);
  newOptions->set("roughness", oldOptions.Roughness);
  newOptions->set("texture-base-color", oldOptions.BaseColorTex);
  newOptions->set("texture-emissive", oldOptions.EmissiveTex);
  newOptions->set("texture-material", oldOptions.ORMTex);
  newOptions->set("texture-normal", oldOptions.NormalTex);

  // Loading but should not
  newOptions->set("cells", oldOptions.Cells);
  newOptions->set("component", oldOptions.Component);
  newOptions->set("fullscreen", oldOptions.FullScreen);
  newOptions->set("resolution", oldOptions.Resolution);
  newOptions->set("scalars", oldOptions.Scalars);
  newOptions->set("up", oldOptions.Up);

  // Rendering/Dynamic
  newOptions->set("axis", oldOptions.Axis);
  newOptions->set("background-color", oldOptions.BackgroundColor);
  newOptions->set("bar", oldOptions.Bar);
  newOptions->set("blur-background", oldOptions.BlurBackground);
  newOptions->set("camera-azimuth-angle", oldOptions.CameraAzimuthAngle);
  newOptions->set("camera-elevation-angle", oldOptions.CameraElevationAngle);
  newOptions->set("camera-focal-point", oldOptions.CameraFocalPoint);
  newOptions->set("camera-position", oldOptions.CameraPosition);
  newOptions->set("camera-view-angle", oldOptions.CameraViewAngle);
  newOptions->set("camera-view-up", oldOptions.CameraViewUp);
  newOptions->set("colormap", oldOptions.Colormap);
  newOptions->set("denoise", oldOptions.Denoise);
  newOptions->set("depth-peeling", oldOptions.DepthPeeling);
  newOptions->set("edges", oldOptions.Edges);
  newOptions->set("filename", oldOptions.Filename);
  newOptions->set("font-file", oldOptions.FontFile);
  newOptions->set("fps", oldOptions.FPS);
  newOptions->set("fxaa", oldOptions.FXAA);
  newOptions->set("grid", oldOptions.Grid);
  newOptions->set("hdri", oldOptions.HDRIFile);
  newOptions->set("inverse", oldOptions.InverseOpacityFunction);
  newOptions->set("metadata", oldOptions.MetaData);
  newOptions->set("point-sprites", oldOptions.PointSprites);
  newOptions->set("range", oldOptions.Range);
  newOptions->set("raytracing", oldOptions.Raytracing);
  newOptions->set("samples", oldOptions.Samples);
  newOptions->set("ssao", oldOptions.SSAO);
  newOptions->set("tone-mapping", oldOptions.ToneMapping);
  newOptions->set("trackball", oldOptions.Trackball);
  newOptions->set("volume", oldOptions.Volume);
}
#endif
