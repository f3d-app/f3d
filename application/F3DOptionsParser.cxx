#include "F3DOptionsParser.h"

#include "F3DConfig.h"
#include "F3DException.h"

#include "cxxopts.hpp"
#include "json.hpp"

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "options.h"

#include <cstdlib>
#include <filesystem>
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

  void GetOptionsFromArgs(
    F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& inputs);
  bool InitializeDictionaryFromConfigFile(const std::string& userConfigFile);

  void SetFilePathForConfigBlock(const std::string& filePath)
  {
    this->FilePathForConfigBlock = filePath;
  }

  enum class HasDefault : bool
  {
    YES = true,
    NO = false
  };

  enum class MayHaveConfig : bool
  {
    YES = true,
    NO = false
  };

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
    const std::string& shortName, const std::string& doc, T& var, HasDefault hasDefault,
    MayHaveConfig mayHaveConfig, const std::string& argHelp = "") const
  {
    bool hasDefaultBool = hasDefault == HasDefault::YES;
    auto val = cxxopts::value<T>(var);
    std::string defaultVal;
    if (hasDefaultBool)
    {
      defaultVal = ConfigurationOptions::ToString(var);
    }

    if (mayHaveConfig == MayHaveConfig::YES)
    {
      hasDefaultBool |= this->GetOptionConfig(longName, defaultVal);
    }

    if (hasDefaultBool)
    {
      val = val->default_value(defaultVal);
    }
    var = {};
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  template<class T>
  void DeclareOptionWithImplicitValue(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, const std::string& implicitValue,
    HasDefault hasDefault, MayHaveConfig mayHaveConfig, const std::string& argHelp = "") const
  {
    bool hasDefaultBool = hasDefault == HasDefault::YES;
    auto val = cxxopts::value<T>(var)->implicit_value(implicitValue);
    std::string defaultVal;
    if (hasDefaultBool)
    {
      defaultVal = ConfigurationOptions::ToString(var);
    }

    if (mayHaveConfig == MayHaveConfig::YES)
    {
      hasDefaultBool |= this->GetOptionConfig(longName, defaultVal);
    }

    if (hasDefaultBool)
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
    cxxopts::Options cxxOptions(F3D::AppName, F3D::AppTitle);
    cxxOptions.positional_help("file1 file2 ...");

    // TODO Rework order of options ?

    // clang-format off
    auto grp0 = cxxOptions.add_options("Applicative");
    this->DeclareOption(grp0, "input", "", "Input file", inputs, HasDefault::NO, MayHaveConfig::NO , "<files>");
    this->DeclareOption(grp0, "output", "", "Render to file", appOptions.Output, HasDefault::NO, MayHaveConfig::NO, "<png file>");
    this->DeclareOption(grp0, "no-background", "", "No background when render to file", appOptions.NoBackground, HasDefault::YES, MayHaveConfig::YES); // Is this OK  ? TODO
    this->DeclareOption(grp0, "help", "h", "Print help");
    this->DeclareOption(grp0, "version", "", "Print version details");
    this->DeclareOption(grp0, "readers-list", "", "Print the list of file types");
    this->DeclareOption(grp0, "config", "", "Read a provided configuration file instead of default one", appOptions.UserConfigFile,  HasDefault::NO, MayHaveConfig::NO , "<file path>");
    this->DeclareOption(grp0, "dry-run", "", "Do not read the configuration file", appOptions.DryRun,  HasDefault::YES, MayHaveConfig::NO );
    this->DeclareOption(grp0, "no-render", "", "Verbose mode without any rendering, only for the first file", appOptions.NoRender,  HasDefault::YES, MayHaveConfig::NO );

    auto grp1 = cxxOptions.add_options("General");
    this->DeclareOption(grp1, "verbose", "", "Enable verbose mode, providing more information about the loaded data in the console output", appOptions.Verbose,  HasDefault::YES, MayHaveConfig::NO );
    this->DeclareOption(grp1, "quiet", "", "Enable quiet mode, which superseed any verbose options and prevent any console output to be generated at all", appOptions.Quiet,  HasDefault::YES, MayHaveConfig::NO );
    this->DeclareOption(grp1, "progress", "", "Show progress bar", options.getAsBoolRef("ui.loader-progress"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "geometry-only", "", "Do not read materials, cameras and lights from file", options.getAsBoolRef("scene.geometry-only"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "up", "", "Up direction", options.getAsStringRef("scene.up-direction"), HasDefault::YES, MayHaveConfig::YES, "[-X|+X|-Y|+Y|-Z|+Z]"); // TODO add a config test, critical
    this->DeclareOption(grp1, "axis", "x", "Show axes", options.getAsBoolRef("interactor.axis"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.getAsBoolRef("ui.grid"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "edges", "e", "Show cell edges", options.getAsBoolRef("scene.show-edges"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "camera-index", "", "Select the camera to use", options.getAsIntRef("scene.camera.index"), HasDefault::YES, MayHaveConfig::YES, "<index>");
    this->DeclareOption(grp1, "trackball", "k", "Enable trackball interaction", options.getAsBoolRef("interactor.trackball"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "animation-index", "", "Select animation to show", options.getAsIntRef("scene.animation.index"), HasDefault::YES, MayHaveConfig::YES, "<index>");
    this->DeclareOption(grp1, "font-file", "", "Path to a FreeType compatible font file", options.getAsStringRef("ui.font-file"), HasDefault::NO, MayHaveConfig::NO, "<file_path>");

    auto grp2 = cxxOptions.add_options("Material");
    this->DeclareOption(grp2, "point-sprites", "o", "Show sphere sprites instead of geometry", options.getAsBoolRef("model.point-sprites.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp2, "point-size", "", "Point size when showing vertices or point sprites", options.getAsDoubleRef("model.point-size"), HasDefault::YES, MayHaveConfig::YES, "<size>");
    this->DeclareOption(grp2, "line-width", "", "Line width when showing edges", options.getAsDoubleRef("model.line-width"), HasDefault::YES, MayHaveConfig::YES, "<width>");
    this->DeclareOption(grp2, "color", "", "Solid color", options.getAsDoubleVectorRef("model.color.rgb"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>");
    this->DeclareOption(grp2, "opacity", "", "Opacity", options.getAsDoubleRef("model.color.opacity"), HasDefault::YES, MayHaveConfig::YES, "<opacity>");
    this->DeclareOption(grp2, "roughness", "", "Roughness coefficient (0.0-1.0)", options.getAsDoubleRef("model.material.roughness"), HasDefault::YES, MayHaveConfig::YES, "<roughness>");
    this->DeclareOption(grp2, "metallic", "", "Metallic coefficient (0.0-1.0)", options.getAsDoubleRef("model.material.metallic"), HasDefault::YES, MayHaveConfig::YES, "<metallic>");
    this->DeclareOption(grp2, "hdri", "", "Path to an image file that will be used as a light source", options.getAsStringRef("window.background.hdri"), HasDefault::NO, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-base-color", "", "Path to a texture file that sets the color of the object", options.getAsStringRef("model.color.texture"), HasDefault::NO, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", options.getAsStringRef("model.material.texture"), HasDefault::NO, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-emissive", "", "Path to a texture file that sets the emitted light of the object", options.getAsStringRef("model.emissive.texture"), HasDefault::NO, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", options.getAsDoubleVectorRef("model.emissive.factor"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>");
    this->DeclareOption(grp2, "texture-normal", "", "Path to a texture file that sets the normal map of the object", options.getAsStringRef("model.normal.texture"), HasDefault::NO, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", options.getAsDoubleRef("model.normal.scale"), HasDefault::YES, MayHaveConfig::YES, "<normalScale>");

    auto grp3 = cxxOptions.add_options("Window");
    this->DeclareOption(grp3, "bg-color", "", "Background color", options.getAsDoubleVectorRef("window.background.color"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>"); // TODO change name ?
    this->DeclareOption(grp3, "resolution", "", "Window resolution", appOptions.Resolution, HasDefault::YES, MayHaveConfig::NO, "<width,height>");
    this->DeclareOption(grp3, "fps", "z", "Display frame per second", options.getAsBoolRef("ui.fps"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "filename", "n", "Display filename", options.getAsBoolRef("ui.filename"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "metadata", "m", "Display file metadata", options.getAsBoolRef("ui.metadata"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "fullscreen", "f", "Full screen", options.getAsBoolRef("window.fullscreen"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "blur-background", "u", "Blur background", options.getAsBoolRef("window.background.blur"), HasDefault::YES, MayHaveConfig::YES);

    auto grp4 = cxxOptions.add_options("Scientific visualization");
    this->DeclareOptionWithImplicitValue(grp4, "scalars", "s", "Color by scalars", options.getAsStringRef("model.scivis.array-name"), std::string(""), HasDefault::YES, MayHaveConfig::YES, "<array_name>");
    this->DeclareOptionWithImplicitValue(grp4, "comp", "y", "Component from the scalar array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", options.getAsIntRef("model.scivis.component"), "-2", HasDefault::YES, MayHaveConfig::YES, "<comp_index>");
    this->DeclareOption(grp4, "cells", "c", "Use a scalar array from the cells", options.getAsBoolRef("model.scivis.cells"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "range", "", "Custom range for the coloring by array", options.getAsDoubleVectorRef("model.scivis.range"), HasDefault::NO, MayHaveConfig::YES, "<min,max>");
    this->DeclareOption(grp4, "bar", "b", "Show scalar bar", options.getAsBoolRef("ui.bar"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "colormap", "", "Specify a custom colormap", options.getAsDoubleVectorRef("model.scivis.colormap"), HasDefault::YES, MayHaveConfig::YES, "<color_list>");
    this->DeclareOption(grp4, "volume", "v", "Show volume if the file is compatible", options.getAsBoolRef("model.volume.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "inverse", "i", "Inverse opacity function for volume rendering", options.getAsBoolRef("model.volume.inverse"), HasDefault::YES, MayHaveConfig::YES);

    auto grpCamera = cxxOptions.add_options("Camera");
    this->DeclareOption(grpCamera, "camera-position", "", "Camera position", appOptions.CameraPosition, HasDefault::NO, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-focal-point", "", "Camera focal point", appOptions.CameraFocalPoint, HasDefault::NO, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-up", "", "Camera view up", appOptions.CameraViewUp, HasDefault::NO, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-angle", "", "Camera view angle (non-zero, in degrees)", appOptions.CameraViewAngle, HasDefault::NO, MayHaveConfig::YES, "<angle>");
    this->DeclareOption(grpCamera, "camera-azimuth-angle", "", "Camera azimuth angle (in degrees)", appOptions.CameraAzimuthAngle, HasDefault::YES, MayHaveConfig::YES, "<angle>");
    this->DeclareOption(grpCamera, "camera-elevation-angle", "", "Camera elevation angle (in degrees)", appOptions.CameraElevationAngle, HasDefault::YES, MayHaveConfig::YES, "<angle>");

#if F3D_MODULE_RAYTRACING
    auto grp5 = cxxOptions.add_options("Raytracing");
    this->DeclareOption(grp5, "raytracing", "r", "Enable raytracing", options.getAsBoolRef("window.raytracing.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp5, "samples", "", "Number of samples per pixel", options.getAsIntRef("window.raytracin.samples"), HasDefault::YES, MayHaveConfig::YES, "<samples>");
    this->DeclareOption(grp5, "denoise", "d", "Denoise the image", options.getAsBoolRef("window.raytracing.denoise"), HasDefault::YES, MayHaveConfig::YES);
#endif

    auto grp6 = cxxOptions.add_options("PostFX (OpenGL)");
    this->DeclareOption(grp6, "depth-peeling", "p", "Enable depth peeling", options.getAsBoolRef("render.effect.depth-peeling"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "ssao", "q", "Enable Screen-Space Ambient Occlusion", options.getAsBoolRef("render.effect.ssao"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "fxaa", "a", "Enable Fast Approximate Anti-Aliasing", options.getAsBoolRef("render.effect.fxaa"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "tone-mapping", "t", "Enable Tone Mapping", options.getAsBoolRef("render.effect.tone-mapping"), HasDefault::YES, MayHaveConfig::YES);

    auto grp7 = cxxOptions.add_options("Testing");
    this->DeclareOption(grp7, "ref", "", "Reference", appOptions.Reference, HasDefault::NO, MayHaveConfig::NO, "<png file>");
    this->DeclareOption(grp7, "ref-threshold", "", "Testing threshold", appOptions.RefThreshold, HasDefault::YES, MayHaveConfig::NO, "<threshold>");
    this->DeclareOption(grp7, "interaction-test-record", "", "Path to an interaction log file to record interactions events to", appOptions.InteractionTestRecordFile, HasDefault::NO, MayHaveConfig::NO, "<file_path>");
    this->DeclareOption(grp7, "interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", appOptions.InteractionTestPlayFile, HasDefault::NO, MayHaveConfig::NO,"<file_path>");
    // clang-format on

    cxxOptions.parse_positional({ "input" });

    auto result = cxxOptions.parse(this->Argc, this->Argv);

    if (result.count("help") > 0)
    {
      this->PrintHelp(cxxOptions);
      throw F3DExNoProcess("help requested");
    }

    if (result.count("version") > 0)
    {
      this->PrintVersion();
      throw F3DExNoProcess("version requested");
    }

    if (result.count("readers-list") > 0)
    {
      this->PrintReadersList();
      throw F3DExNoProcess("reader list requested");
    }
  }
  catch (const cxxopts::OptionException& ex)
  {
    f3d::log::error("Error parsing options: ", ex.what());
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
  // clang-format off
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
  for (const auto& key : f3d::interactor::getDefaultInteractionsInfo())
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
  f3d::log::setUseColoring(false);
  f3d::log::info(F3D::AppName + " " + F3D::AppVersion + "\n");
  f3d::log::info(F3D::AppTitle);
  auto libInfo = f3d::engine::getLibInfo();
  f3d::log::info("Version: " + libInfo["Version"] + ".");
  f3d::log::info("Build date: " + libInfo["Build date"] + ".");
  f3d::log::info("Build system: " + libInfo["Build system"] + ".");
  f3d::log::info("Compiler: " + libInfo["Compiler"] + ".");
  f3d::log::info("External rendering module: " + libInfo["External rendering module"] + ".");
  f3d::log::info("Raytracing module: " + libInfo["Raytracing module"] + ".");
  f3d::log::info("Exodus module: " + libInfo["Exodus module"] + ".");
  f3d::log::info("OpenCASCADE module: " + libInfo["OpenCASCADE module"] + ".");
  f3d::log::info("Assimp module: " + libInfo["Assimp module"] + ".");
  f3d::log::info("Alembic module: " + libInfo["Alembic module"] + ".");
  f3d::log::info("VTK version: " + libInfo["VTK version"] + ".");
  f3d::log::info(libInfo["Copyright_previous"] + ".");
  f3d::log::info(libInfo["Copyright"] + ".");
  f3d::log::info("License " + libInfo["License"] + ".");
  f3d::log::info("By " + libInfo["Authors"] + ".");
  f3d::log::setUseColoring(true);
  f3d::log::waitForUser();
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintReadersList()
{
  size_t nameColSize = 0;
  size_t extsColSize = 0;
  size_t mimeColSize = 0;
  size_t descColSize = 0;

  std::vector<f3d::engine::readerInformation> readersInfo = f3d::engine::getReadersInfo();
  if (readersInfo.empty())
  {
    f3d::log::warn("No registered reader found!");
    return;
  }
  // Compute the size of the 3 columns
  for (const auto& reader : readersInfo)
  {
    // sanity check
    if (reader.extensions.size() < reader.mimetypes.size())
    {
      f3d::log::error(reader.name, "More mimetypes than extensions, unexpected.");
      return;
    }

    nameColSize = std::max(nameColSize, reader.name.length());
    descColSize = std::max(descColSize, reader.description.length());

    for (const auto& ext : reader.extensions)
    {
      extsColSize = std::max(extsColSize, ext.length());
    }
    for (const auto& mime : reader.mimetypes)
    {
      mimeColSize = std::max(mimeColSize, mime.length());
    }
  }
  nameColSize++;
  extsColSize++;
  mimeColSize++;
  descColSize++;

  std::string separator = std::string(nameColSize + extsColSize + descColSize + mimeColSize, '-');

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(descColSize)
             << "Description" << std::setw(extsColSize) << "Exts" << std::setw(mimeColSize)
             << "Mime-types";
  f3d::log::info(headerLine.str());
  f3d::log::info(separator);

  for (const auto& reader : readersInfo)
  {
    for (size_t i = 0; i < reader.extensions.size(); i++)
    {
      std::stringstream readerLine;
      if (i == 0)
      {
        readerLine << std::left << std::setw(nameColSize) << reader.name << std::setw(descColSize)
                   << reader.description;
      }
      else
      {
        readerLine << std::left << std::setw(nameColSize + descColSize) << " ";
      }

      readerLine << std::setw(extsColSize) << reader.extensions[i];

      if (i < reader.mimetypes.size())
      {
        readerLine << std::setw(mimeColSize) << reader.mimetypes[i];
      }

      f3d::log::info(readerLine.str());
    }
    f3d::log::info(separator);
  }
  f3d::log::waitForUser();
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
void F3DOptionsParser::GetOptionsFromConfigFile(
  const std::string& filePath, F3DAppOptions& appOptions, f3d::options& options)
{
  this->ConfigOptions->SetFilePathForConfigBlock(filePath);
  std::vector<std::string> dummyFiles;
  return this->ConfigOptions->GetOptionsFromArgs(appOptions, options, dummyFiles);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::GetOptionsFromCommandLine(
  F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& files)
{
  this->ConfigOptions->SetFilePathForConfigBlock("");
  return this->ConfigOptions->GetOptionsFromArgs(appOptions, options, files);
}
