#include "F3DOptions.h"

#include "F3DLog.h"
#include "F3DException.h"
#include "F3DReaderFactory.h"

#include <vtk_jsoncpp.h>
#include <vtksys/SystemTools.hxx>
#include <vtkVersion.h>

#include <fstream>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>

#include "cxxopts.hpp"

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

  void SetFilePathForConfigBlock(const std::string& filePath) { this->FilePathForConfigBlock = filePath; }

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

  static std::string ToString(bool currValue)
  {
    return currValue ? "true" : "false";
  }

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
    const std::string& shortName, const std::string& doc, T& var, bool hasDefault = true, bool mayHaveConfig = true,
    const std::string& argHelp = "") const
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

  void PrintHelpPair(const std::string& key, const std::string& help, int keyWidth = 10, int helpWidth = 70);
  void PrintHelp(cxxopts::Options& cxxOptions);
  void PrintVersion();
  void PrintReadersList();
  void PrintExtensionsList();

private:
  int Argc;
  char** Argv;

  std::string FilePathForConfigBlock;

  using Dictionnary = std::map<std::string, std::map<std::string, std::string>>;
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
    this->DeclareOption(grp1, "extensions-list", "", "Print the list of supported extensions");
    this->DeclareOption(grp1, "verbose", "", "Enable verbose mode, providing more information about the loaded data in the console output", options.Verbose);
    this->DeclareOption(grp1, "no-render", "", "Verbose mode without any rendering, only for the first file", options.NoRender);
    this->DeclareOption(grp1, "quiet", "", "Enable quiet mode, which superseed any verbose options and prevent any console output to be generated at all", options.Quiet);
    this->DeclareOption(grp1, "axis", "x", "Show axes", options.Axis);
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.Grid);
    this->DeclareOption(grp1, "edges", "e", "Show cell edges", options.Edges);
    this->DeclareOption(grp1, "trackball", "k", "Enable trackball interaction", options.Trackball);
    this->DeclareOption(grp1, "progress", "", "Show progress bar", options.Progress);
    this->DeclareOption(grp1, "up", "", "Up direction", options.Up, true, "[-X|+X|-Y|+Y|-Z|+Z]");
    this->DeclareOption(grp1, "animation-index", "", "Select animation to show", options.AnimationIndex, true, true, "<index>");
#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 0, 20210228)
    this->DeclareOption(grp1, "camera-index", "", "Select the camera to use", options.CameraIndex, true, true, "<index>");
#endif
    this->DeclareOption(grp1, "geometry-only", "", "Do not read materials, cameras and lights from file", options.GeometryOnly);
    this->DeclareOption(grp1, "dry-run", "", "Do not read the configuration file", options.DryRun, true, false);
    this->DeclareOption(grp1, "config", "", "Read a provided configuration file instead of default one", options.UserConfigFile, false, false, "<file path>");

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
    this->DeclareOption(grp3, "resolution", "", "Window resolution", options.WindowSize, true, true, "<width,height>");
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
    this->DeclareOption(grp4, "colormap", "", "Specify a custom colormap", options.LookupPoints,
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

    int argc = this->Argc;
    auto result = cxxOptions.parse(argc, this->Argv);

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

    if (result.count("extensions-list") > 0)
    {
      this->PrintExtensionsList();
      throw F3DExNoProcess();
    }
  }
  catch (const cxxopts::OptionException& e)
  {
    F3DLog::Print(F3DLog::Severity::Error, "Error parsing options: ", e.what());
    throw;
  }
  return options;
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintHelpPair(const std::string& key, const std::string& help, int keyWidth, int helpWidth)
{
  std::stringstream ss;
  ss << "  " << std::left << std::setw(keyWidth) << key << " " << std::setw(helpWidth) << help;
  F3DLog::Print(F3DLog::Severity::Info, ss.str());
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintHelp(cxxopts::Options& cxxOptions)
{
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

  F3DLog::SetUseColoring(false);
  F3DLog::Print(F3DLog::Severity::Info, cxxOptions.help());
  F3DLog::Print(F3DLog::Severity::Info, "Keys:");
  for (const auto& key : keys)
  {
    this->PrintHelpPair(key.first, key.second);
  }

  F3DLog::Print(F3DLog::Severity::Info, "\nExamples:");
  for (const auto& example : examples)
  {
    this->PrintHelpPair(example.first, example.second, 50);
  }
  F3DLog::Print(F3DLog::Severity::Info, "\nReport bugs to https://github.com/f3d-app/f3d/issues");
  F3DLog::SetUseColoring(true);
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintVersion()
{
  std::string version = f3d::AppName + " " + f3d::AppVersion + "\n\n";

  version += f3d::AppTitle;
  version += "\nVersion: ";
  version += f3d::AppVersion;
  version += "\nBuild date: ";
  version += f3d::AppBuildDate;
  version += "\nSystem: ";
  version += f3d::AppBuildSystem;
  version += "\nCompiler: ";
  version += f3d::AppCompiler;
  version += "\nRayTracing module: ";
#if F3D_MODULE_RAYTRACING
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nExodus module: ";
#if F3D_MODULE_EXODUS
  version += "ON";
#else
  version += "OFF";
#endif
  version += "\nOpenCASCADE module: ";
#if F3D_MODULE_OCCT
  version += F3D_OCCT_VERSION;
#if F3D_MODULE_OCCT_XCAF
  version += " (full support)";
#else
  version += " (no metadata)";
#endif
#else
  version += "OFF";
#endif
  version += "\nAssimp module: ";
#if F3D_MODULE_ASSIMP
  version += F3D_ASSIMP_VERSION;
#else
  version += "OFF";
#endif
  version += "\nVTK version: ";
  version += std::string(VTK_VERSION) + std::string(" (build ") +
    std::to_string(VTK_BUILD_VERSION) + std::string(")");

  version += "\n\nCopyright (C) 2021 Kitware SAS.";
  version += "\nLicense BSD-3-Clause.";
  version += "\nWritten by Michael Migliore, Mathieu Westphal and Joachim Pouderoux.";

  F3DLog::SetUseColoring(false);
  F3DLog::Print(F3DLog::Severity::Info, version);
  F3DLog::SetUseColoring(true);
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintReadersList()
{
  size_t nameColSize = 0;
  size_t extsColSize = 0;
  size_t descColSize = 0;

  const auto& readers = F3DReaderFactory::GetInstance()->GetReaders();
  if (readers.empty())
  {
    F3DLog::Print(F3DLog::Severity::Warning, "No registered reader found!");
    return;
  }
  // Compute the size of the 3 columns
  for (const auto& reader : readers)
  {
    nameColSize = std::max(nameColSize, reader->GetName().length());
    descColSize = std::max(descColSize, reader->GetLongDescription().length());
    auto exts = reader->GetExtensions();
    size_t extLen = 0;
    int cnt = 0;
    for (const auto& ext : exts)
    {
      if (cnt++ > 0)
      {
        extLen++;
      }
      extLen += ext.length();
    }
    extsColSize = std::max(extsColSize, extLen);
  }
  nameColSize++;
  extsColSize++;
  descColSize++;

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(extsColSize)
             << "Extensions" << std::setw(descColSize) << "Description";
  F3DLog::Print(F3DLog::Severity::Info, headerLine.str());
  F3DLog::Print(F3DLog::Severity::Info, std::string(nameColSize + extsColSize + descColSize, '-'));

  for (const auto& reader : readers)
  {
    std::stringstream readerLine;
    readerLine << std::left << std::setw(nameColSize) << reader->GetName()
               << std::setw(extsColSize);
    auto exts = reader->GetExtensions();
    unsigned int cnt = 0;
    std::string extLine;
    for (const auto& ext : exts)
    {
      if (cnt++ > 0)
      {
        extLine += ";";
      }
      extLine += ext;
    }
    readerLine << extLine << std::setw(descColSize) << reader->GetLongDescription();
    F3DLog::Print(F3DLog::Severity::Info, readerLine.str());
  }
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintExtensionsList()
{
  const auto& readers = F3DReaderFactory::GetInstance()->GetReaders();
  if (readers.size() == 0)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "No registered reader found!");
    return;
  }
  std::stringstream extList;
  unsigned int cnt = 0;
  for (const auto& reader : readers)
  {
    auto exts = reader->GetExtensions();
    for (const auto& ext : exts)
    {
      if (cnt++ > 0)
      {
        extList << ";";
      }
      extList << ext;
    }
  }
  F3DLog::Print(F3DLog::Severity::Info, extList.str());
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

  std::ifstream file;
  file.open(configFilePath.c_str());

  if (!file.is_open())
  {
    F3DLog::Print(
      F3DLog::Severity::Error, "Unable to open the configuration file ", configFilePath);
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
    F3DLog::Print(
      F3DLog::Severity::Error, "Unable to parse the configuration file ", configFilePath);
    F3DLog::Print(F3DLog::Severity::Error, errs);
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
