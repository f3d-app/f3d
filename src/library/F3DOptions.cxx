#include "F3DOptions.h"

#include "cxxopts.hpp"

#include "F3DException.h"
#include "f3d_engine.h"
#include "f3d_log.h"
#include "f3d_options.h"

#include <vtk_jsoncpp.h>
#include <vtksys/SystemTools.hxx>

#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>

//----------------------------------------------------------------------------
class ConfigurationOptions
{
public:
  ConfigurationOptions(int argc, char** argv)
    : Argc(argc)
    , Argv(argv)
  {
  }

  F3DOptions GetOptionsFromArgs(std::vector<std::string>& inputs);
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

  std::string GetBinarySettingsDirectory();
  std::string GetSettingsFilePath();

  static std::string GetSystemSettingsDirectory();
  static std::string GetUserSettingsDirectory();

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
F3DOptions ConfigurationOptions::GetOptionsFromArgs(std::vector<std::string>& inputs)
{
  F3DOptions options;
  try
  {
    cxxopts::Options cxxOptions(f3d::AppName, f3d::AppTitle);
    cxxOptions.positional_help("file1 file2 ...");

    // clang-format off
    auto grp1 = cxxOptions.add_options();
    this->DeclareOption(grp1, "input", "", "Input file", inputs, false, false, "<files>");
    this->DeclareOption(grp1, "output", "", "Render to file", options.Output, false, false,"<png file>");
    this->DeclareOption(grp1, "no-background", "", "No background when render to file", options.NoBackground);
    this->DeclareOption(grp1, "help", "h", "Print help");
    this->DeclareOption(grp1, "version", "", "Print version details");
    this->DeclareOption(grp1, "readers-list", "", "Print the list of file types");
    this->DeclareOption(grp1, "verbose", "", "Enable verbose mode, providing more information about the loaded data in the console output", options.Verbose);
    this->DeclareOption(grp1, "no-render", "", "Verbose mode without any rendering, only for the first file", options.NoRender, true, false);
    this->DeclareOption(grp1, "quiet", "", "Enable quiet mode, which superseed any verbose options and prevent any console output to be generated at all", options.Quiet);
    this->DeclareOption(grp1, "axis", "x", "Show axes", options.Axis);
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.Grid);
    this->DeclareOption(grp1, "edges", "e", "Show cell edges", options.Edges);
    this->DeclareOption(grp1, "trackball", "k", "Enable trackball interaction", options.Trackball);
    this->DeclareOption(grp1, "progress", "", "Show progress bar", options.Progress);
    this->DeclareOption(grp1, "up", "", "Up direction", options.Up, true, "[-X|+X|-Y|+Y|-Z|+Z]");
    this->DeclareOption(grp1, "animation-index", "", "Select animation to show", options.AnimationIndex, true, true, "<index>");
    this->DeclareOption(grp1, "camera-index", "", "Select the camera to use", options.CameraIndex, true, true, "<index>");
    this->DeclareOption(grp1, "geometry-only", "", "Do not read materials, cameras and lights from file", options.GeometryOnly);
    this->DeclareOption(grp1, "dry-run", "", "Do not read the configuration file", options.DryRun, true, false);
    this->DeclareOption(grp1, "config", "", "Read a provided configuration file instead of default one", options.UserConfigFile, false, false, "<file path>");
    this->DeclareOption(grp1, "font-file", "", "Path to a FreeType compatible font file", options.FontFile, false, false, "<file_path>");

    auto grp2 = cxxOptions.add_options("Material");
    this->DeclareOption(grp2, "point-sprites", "o", "Show sphere sprites instead of geometry", options.PointSprites);
    this->DeclareOption(grp2, "point-size", "", "Point size when showing vertices or point sprites", options.PointSize, true, true, "<size>");
    this->DeclareOption(grp2, "line-width", "", "Line width when showing edges", options.LineWidth, true, true, "<width>");
    this->DeclareOption(grp2, "color", "", "Solid color", options.SolidColor, true, true, "<R,G,B>");
    this->DeclareOption(grp2, "opacity", "", "Opacity", options.Opacity, true, true, "<opacity>");
    this->DeclareOption(grp2, "roughness", "", "Roughness coefficient (0.0-1.0)", options.Roughness, true, true, "<roughness>");
    this->DeclareOption(grp2, "metallic", "", "Metallic coefficient (0.0-1.0)", options.Metallic, true, true, "<metallic>");
    this->DeclareOption(grp2, "hdri", "", "Path to an image file that will be used as a light source", options.HDRIFile, false, true, "<file path>");
    this->DeclareOption(grp2, "texture-base-color", "", "Path to a texture file that sets the color of the object", options.BaseColorTex, false, true, "<file path>");
    this->DeclareOption(grp2, "texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", options.ORMTex, false, true, "<file path>");
    this->DeclareOption(grp2, "texture-emissive", "", "Path to a texture file that sets the emitted light of the object", options.EmissiveTex, false, true, "<file path>");
    this->DeclareOption(grp2, "emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", options.EmissiveFactor, true, true, "<R,G,B>");
    this->DeclareOption(grp2, "texture-normal", "", "Path to a texture file that sets the normal map of the object", options.NormalTex, false, true, "<file path>");
    this->DeclareOption(grp2, "normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", options.NormalScale, true, true, "<normalScale>");

    auto grp3 = cxxOptions.add_options("Window");
    this->DeclareOption(grp3, "bg-color", "", "Background color", options.BackgroundColor, true, true, "<R,G,B>");
    this->DeclareOption(grp3, "resolution", "", "Window resolution", options.Resolution, true, true, "<width,height>");
    this->DeclareOption(grp3, "fps", "z", "Display frame per second", options.FPS);
    this->DeclareOption(grp3, "filename", "n", "Display filename", options.Filename);
    this->DeclareOption(grp3, "metadata", "m", "Display file metadata", options.MetaData);
    this->DeclareOption(grp3, "fullscreen", "f", "Full screen", options.FullScreen);
    this->DeclareOption(grp3, "blur-background", "u", "Blur background", options.BlurBackground);

    auto grp4 = cxxOptions.add_options("Scientific visualization");
    this->DeclareOptionWithImplicitValue(grp4, "scalars", "s", "Color by scalars", options.Scalars, std::string(""), true, true, "<array_name>");
    this->DeclareOptionWithImplicitValue(grp4, "comp", "y", "Component from the scalar array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", options.Component, "-2", true, true, "<comp_index>");
    this->DeclareOption(grp4, "cells", "c", "Use a scalar array from the cells", options.Cells);
    this->DeclareOption(grp4, "range", "", "Custom range for the coloring by array", options.Range, false, true, "<min,max>");
    this->DeclareOption(grp4, "bar", "b", "Show scalar bar", options.Bar);
    this->DeclareOption(grp4, "colormap", "", "Specify a custom colormap", options.Colormap,
      true, "<color_list>");
    this->DeclareOption(grp4, "volume", "v", "Show volume if the file is compatible", options.Volume);
    this->DeclareOption(grp4, "inverse", "i", "Inverse opacity function for volume rendering", options.InverseOpacityFunction);

    auto grpCamera = cxxOptions.add_options("Camera");
    this->DeclareOption(grpCamera, "camera-position", "", "Camera position", options.CameraPosition, false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-focal-point", "", "Camera focal point", options.CameraFocalPoint, false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-up", "", "Camera view up", options.CameraViewUp, false, true, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-angle", "", "Camera view angle (non-zero, in degrees)", options.CameraViewAngle, false, true, "<angle>");
    this->DeclareOption(grpCamera, "camera-azimuth-angle", "", "Camera azimuth angle (in degrees)", options.CameraAzimuthAngle, true, true, "<angle>");
    this->DeclareOption(grpCamera, "camera-elevation-angle", "", "Camera elevation angle (in degrees)", options.CameraElevationAngle, true, true, "<angle>");

#if F3D_MODULE_RAYTRACING
    auto grp5 = cxxOptions.add_options("Raytracing");
    this->DeclareOption(grp5, "raytracing", "r", "Enable raytracing", options.Raytracing);
    this->DeclareOption(grp5, "samples", "", "Number of samples per pixel", options.Samples, true, true, "<samples>");
    this->DeclareOption(grp5, "denoise", "d", "Denoise the image", options.Denoise);
#endif

    auto grp6 = cxxOptions.add_options("PostFX (OpenGL)");
    this->DeclareOption(grp6, "depth-peeling", "p", "Enable depth peeling", options.DepthPeeling);
    this->DeclareOption(grp6, "ssao", "q", "Enable Screen-Space Ambient Occlusion", options.SSAO);
    this->DeclareOption(grp6, "fxaa", "a", "Enable Fast Approximate Anti-Aliasing", options.FXAA);
    this->DeclareOption(grp6, "tone-mapping", "t", "Enable Tone Mapping", options.ToneMapping);

    auto grp7 = cxxOptions.add_options("Testing");
    this->DeclareOption(grp7, "ref", "", "Reference", options.Reference, false, false, "<png file>");
    this->DeclareOption(grp7, "ref-threshold", "", "Testing threshold", options.RefThreshold, true, false, "<threshold>");
    this->DeclareOption(grp7, "interaction-test-record", "", "Path to an interaction log file to record interactions events to", options.InteractionTestRecordFile, false, false, "<file_path>");
    this->DeclareOption(grp7, "interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", options.InteractionTestPlayFile, false, false,"<file_path>");
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
  return options;
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
    configFilePath = this->GetSettingsFilePath();
  }
  if (configFilePath.empty())
  {
    return false;
  }

  configFilePath = vtksys::SystemTools::CollapseFullPath(configFilePath);
  std::ifstream file;
  file.open(configFilePath.c_str());

  if (!file.is_open())
  {
    f3d::log::error("Unable to open the configuration file: ", configFilePath);
    return false;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  std::string errs;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  bool success = Json::parseFromStream(builder, file, &root, &errs);
  if (!success)
  {
    f3d::log::error("Unable to parse the configuration file ", configFilePath);
    f3d::log::error(errs);
    return false;
  }

  for (auto const& id : root.getMemberNames())
  {
    const Json::Value node = root[id];
    std::map<std::string, std::string> localDic;
    for (auto const& nl : node.getMemberNames())
    {
      const Json::Value v = node[nl];
      localDic[nl] = v.asString();
    }
    this->ConfigDic[id] = localDic;
  }

  return true;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetUserSettingsDirectory()
{
  std::string applicationName = "f3d";
#if defined(_WIN32)
  const char* appData = vtksys::SystemTools::GetEnv("APPDATA");
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
  const char* xdgConfigHome = vtksys::SystemTools::GetEnv("XDG_CONFIG_HOME");
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
    const char* home = vtksys::SystemTools::GetEnv("HOME");
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
std::string ConfigurationOptions::GetSystemSettingsDirectory()
{
  std::string directoryPath = "";
// No support implemented for system wide settings on Windows yet
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
std::string ConfigurationOptions::GetBinarySettingsDirectory()
{
  std::string directoryPath = "";
  std::string errorMsg, programFilePath;
  if (vtksys::SystemTools::FindProgramPath(this->Argv[0], programFilePath, errorMsg))
  {
    // resolve symlinks
    programFilePath = vtksys::SystemTools::GetRealPath(programFilePath);
    directoryPath = vtksys::SystemTools::GetProgramPath(programFilePath);
    std::string separator;
#if defined(_WIN32)
    separator = "\\";
    if (directoryPath[directoryPath.size() - 1] != separator[0])
    {
      directoryPath.append(separator);
    }
#else
    separator = "/";
    directoryPath += separator;
#endif
    directoryPath += "..";
#if F3D_MACOS_BUNDLE
    if (vtksys::SystemTools::FileExists(directoryPath + "/Resources"))
    {
      directoryPath += "/Resources";
    }
#endif
    directoryPath = vtksys::SystemTools::CollapseFullPath(directoryPath);
    directoryPath += separator;
  }
  return directoryPath;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetSettingsFilePath()
{
  std::string fileName = "config.json";
  std::string filePath = ConfigurationOptions::GetUserSettingsDirectory() + fileName;
  if (!vtksys::SystemTools::FileExists(filePath))
  {
    filePath = this->GetBinarySettingsDirectory() + fileName;
    if (!vtksys::SystemTools::FileExists(filePath))
    {
      filePath = ConfigurationOptions::GetSystemSettingsDirectory() + fileName;
      if (!vtksys::SystemTools::FileExists(filePath))
      {
        filePath = "";
      }
    }
  }
  return filePath;
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
F3DOptions F3DOptionsParser::GetOptionsFromConfigFile(const std::string& filePath)
{
  this->ConfigOptions->SetFilePathForConfigBlock(filePath);
  std::vector<std::string> dummy;
  return this->ConfigOptions->GetOptionsFromArgs(dummy);
}

//----------------------------------------------------------------------------
F3DOptions F3DOptionsParser::GetOptionsFromCommandLine(std::vector<std::string>& files)
{
  this->ConfigOptions->SetFilePathForConfigBlock("");
  return this->ConfigOptions->GetOptionsFromArgs(files);
}

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
