#include "F3DOptionsParser.h"

#include "F3DConfig.h"
#include "F3DConfigFileTools.h"
#include "F3DException.h"
#include "F3DSystemTools.h"

#include "cxxopts.hpp"
#include "nlohmann/json.hpp"

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "options.h"
#include "utils.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

//----------------------------------------------------------------------------
class ConfigurationOptions
{
public:
  ConfigurationOptions(int argc, char** argv)
    : Argc(argc)
    , Argv(argv)
  {
    this->ExecutableName = argc > 0 && argv[0][0] ? argv[0] : "f3d";
  }

  void GetOptions(F3DAppOptions& appOptions, f3d::options& options,
    std::vector<std::string>& inputs, std::string filePathForConfigBlock = "",
    bool allOptionsInitialized = false, bool parseCommandLine = true);
  bool InitializeDictionaryFromConfigFile(const std::string& userConfigFile);
  void LoadPlugins(const F3DAppOptions& appOptions) const;
  std::vector<std::string> GetPluginSearchPaths() const;

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

  enum class HasImplicitValue : bool
  {
    YES = true,
    NO = false
  };

protected:
  bool GetOptionConfig(const std::string& option, std::string& configValue) const
  {
    bool ret = false;
    if (this->FilePathForConfigBlock.empty())
    {
      auto localIt = this->GlobalConfigDicEntry.find(option);
      if (localIt != this->GlobalConfigDicEntry.end())
      {
        configValue = localIt->second;
        ret = true;
      }
    }
    else
    {
      for (auto const& it : this->ConfigDic)
      {
        std::regex re(it.first, std::regex_constants::icase);
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
    const std::string& shortName, const std::string& doc)
  {
    this->AllLongOptions.push_back(longName);
    group(this->CollapseName(longName, shortName), doc);
  }

  template<class T>
  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, HasDefault hasDefault,
    MayHaveConfig mayHaveConfig, const std::string& argHelp = "",
    HasImplicitValue hasImplicitValue = HasImplicitValue::NO, const std::string& implicitValue = "")
  {
    bool hasDefaultBool = hasDefault == HasDefault::YES;
    auto val = cxxopts::value<T>(var);

    if (hasImplicitValue == HasImplicitValue::YES)
    {
      val->implicit_value(implicitValue);
    }

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
    this->AllLongOptions.push_back(longName);
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  void PrintHelpPair(
    const std::string& key, const std::string& help, int keyWidth = 10, int helpWidth = 70);
  void PrintHelp(const cxxopts::Options& cxxOptions);
  void PrintVersion();
  void PrintReadersList();
  void PrintPluginsScan();

  std::pair<std::string, int> GetClosestOption(const std::string& option)
  {
    std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };

    for (const std::string& name : this->AllLongOptions)
    {
      int distance = f3d::utils::textDistance(name, option);
      if (distance < ret.second)
      {
        ret = { name, distance };
      }
    }

    return ret;
  }

private:
  int Argc;
  char** Argv;

  std::string FilePathForConfigBlock;

  using DictionaryEntry = std::map<std::string, std::string>;
  using Dictionary = std::map<std::string, DictionaryEntry>;
  DictionaryEntry GlobalConfigDicEntry;
  Dictionary ConfigDic;
  std::string ExecutableName;
  std::vector<std::string> AllLongOptions;
};

//----------------------------------------------------------------------------
std::vector<std::string> ConfigurationOptions::GetPluginSearchPaths() const
{
  // Recover F3D_PLUGINS_PATH first
  auto searchPaths = F3DSystemTools::GetVectorEnvironnementVariable("F3D_PLUGINS_PATH");
#if F3D_MACOS_BUNDLE
  return searchPaths;
#else
  // Add a executable related path
  auto tmpPath = F3DSystemTools::GetApplicationPath();
  tmpPath = tmpPath.parent_path().parent_path();
  tmpPath /= F3D::PluginsInstallDir;
  searchPaths.push_back(tmpPath.string());
  return searchPaths;
#endif
}

//----------------------------------------------------------------------------
void ConfigurationOptions::LoadPlugins(const F3DAppOptions& appOptions) const
{
  try
  {
    f3d::engine::autoloadPlugins();

    for (const std::string& plugin : appOptions.Plugins)
    {
      if (!plugin.empty())
      {
        f3d::engine::loadPlugin(plugin, this->GetPluginSearchPaths());
      }
    }
  }
  catch (const f3d::engine::plugin_exception& e)
  {
    f3d::log::warn("Plugin failed to load: ", e.what());
  }
}

//----------------------------------------------------------------------------
void ConfigurationOptions::PrintPluginsScan()
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
void ConfigurationOptions::GetOptions(F3DAppOptions& appOptions, f3d::options& options,
  std::vector<std::string>& inputs, std::string filePathForConfigBlock, bool allOptionsInitialized,
  bool parseCommandLine)
{
  this->FilePathForConfigBlock = std::move(filePathForConfigBlock);

  // When parsing multiple times, hasDefault should be forced to yes after the first pass as all
  // options are expected to be already initialized, which means they have a "default" in the
  // cxxopts sense.
  HasDefault LocalHasDefaultNo = allOptionsInitialized ? HasDefault::YES : HasDefault::NO;

  try
  {
    cxxopts::Options cxxOptions(this->ExecutableName, F3D::AppTitle);
    cxxOptions.positional_help("file1 file2 ...");

    // clang-format off
    auto grp0 = cxxOptions.add_options("Applicative");
    this->DeclareOption(grp0, "input", "", "Input files", inputs, LocalHasDefaultNo, MayHaveConfig::YES , "<files>");
    this->DeclareOption(grp0, "output", "", "Render to file", appOptions.Output, LocalHasDefaultNo, MayHaveConfig::YES, "<png file>");
    this->DeclareOption(grp0, "no-background", "", "No background when render to file", appOptions.NoBackground, HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp0, "help", "h", "Print help");
    this->DeclareOption(grp0, "version", "", "Print version details");
    this->DeclareOption(grp0, "readers-list", "", "Print the list of readers");
    this->DeclareOption(grp0, "config", "", "Specify the configuration file to use. absolute/relative path or filename/filestem to search in configuration file locations.", appOptions.UserConfigFile,  LocalHasDefaultNo, MayHaveConfig::NO , "<filePath/filename/fileStem>");
    this->DeclareOption(grp0, "dry-run", "", "Do not read the configuration file", appOptions.DryRun,  HasDefault::YES, MayHaveConfig::NO );
    this->DeclareOption(grp0, "no-render", "", "Verbose mode without any rendering, only for the first file", appOptions.NoRender,  HasDefault::YES, MayHaveConfig::YES );
    this->DeclareOption(grp0, "max-size", "", "Maximum size in Mib of a file to load, negative value means unlimited", appOptions.MaxSize,  HasDefault::YES, MayHaveConfig::YES, "<size in Mib>");
    this->DeclareOption(grp0, "load-plugins", "", "List of plugins to load separated with a comma", appOptions.Plugins, LocalHasDefaultNo, MayHaveConfig::YES, "<paths or names>");
    this->DeclareOption(grp0, "scan-plugins", "", "Scan some directories for plugins (result can be incomplete)");

    auto grp1 = cxxOptions.add_options("General");
    this->DeclareOption(grp1, "verbose", "", "Enable verbose mode, providing more information about the loaded data in the console output", appOptions.Verbose,  HasDefault::YES, MayHaveConfig::YES );
    this->DeclareOption(grp1, "quiet", "", "Enable quiet mode, which supersede any verbose options and prevent any console output to be generated at all", appOptions.Quiet,  HasDefault::YES, MayHaveConfig::YES );
    this->DeclareOption(grp1, "progress", "", "Show progress bar", options.getAsBoolRef("ui.loader-progress"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "geometry-only", "", "Do not read materials, cameras and lights from file", appOptions.GeometryOnly, HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "group-geometries", "", "When opening multiple files, show them all in the same scene. Force geometry-only. The configuration file for the first file will be loaded.", appOptions.GroupGeometries, HasDefault::YES, MayHaveConfig::NO);
    this->DeclareOption(grp1, "up", "", "Up direction", options.getAsStringRef("scene.up-direction"), HasDefault::YES, MayHaveConfig::YES, "{-X, +X, -Y, +Y, -Z, +Z}");
    this->DeclareOption(grp1, "axis", "x", "Show axes", options.getAsBoolRef("interactor.axis"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.getAsBoolRef("render.grid.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "grid-absolute", "", "Position grid at the absolute origin instead of below the model", options.getAsBoolRef("render.grid.absolute"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "grid-unit", "", "Size of grid unit square", options.getAsDoubleRef("render.grid.unit"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "grid-subdivisions", "", "Number of grid subdivisions", options.getAsIntRef("render.grid.subdivisions"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "edges", "e", "Show cell edges", options.getAsBoolRef("render.show-edges"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "camera-index", "", "Select the camera to use", options.getAsIntRef("scene.camera.index"), HasDefault::YES, MayHaveConfig::YES, "<index>");
    this->DeclareOption(grp1, "trackball", "k", "Enable trackball interaction", options.getAsBoolRef("interactor.trackball"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "invert-zoom", "", "Invert zoom direction with right mouse click", options.getAsBoolRef("interactor.invert-zoom"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp1, "animation-index", "", "Select animation to show", options.getAsIntRef("scene.animation.index"), HasDefault::YES, MayHaveConfig::YES, "<index>");
    this->DeclareOption(grp1, "animation-speed-factor", "", "Set animation speed factor", options.getAsDoubleRef("scene.animation.speed-factor"), HasDefault::YES, MayHaveConfig::YES, "<factor>");
    this->DeclareOption(grp1, "animation-time", "", "Set animation time to load", options.getAsDoubleRef("scene.animation.time"), HasDefault::YES, MayHaveConfig::YES, "<time>");
    this->DeclareOption(grp1, "animation-frame-rate", "", "Set animation frame rate when playing animation interactively", options.getAsDoubleRef("scene.animation.frame-rate"), HasDefault::YES, MayHaveConfig::YES, "<frame rate>");
    this->DeclareOption(grp1, "font-file", "", "Path to a FreeType compatible font file", options.getAsStringRef("ui.font-file"), LocalHasDefaultNo, MayHaveConfig::NO, "<file_path>");

    auto grp2 = cxxOptions.add_options("Material");
    this->DeclareOption(grp2, "point-sprites", "o", "Show sphere sprites instead of geometry", options.getAsBoolRef("model.point-sprites.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp2, "point-size", "", "Point size when showing vertices or point sprites", options.getAsDoubleRef("render.point-size"), HasDefault::YES, MayHaveConfig::YES, "<size>");
    this->DeclareOption(grp2, "line-width", "", "Line width when showing edges", options.getAsDoubleRef("render.line-width"), HasDefault::YES, MayHaveConfig::YES, "<width>");
    this->DeclareOption(grp2, "color", "", "Solid color", options.getAsDoubleVectorRef("model.color.rgb"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>");
    this->DeclareOption(grp2, "opacity", "", "Opacity", options.getAsDoubleRef("model.color.opacity"), HasDefault::YES, MayHaveConfig::YES, "<opacity>");
    this->DeclareOption(grp2, "roughness", "", "Roughness coefficient (0.0-1.0)", options.getAsDoubleRef("model.material.roughness"), HasDefault::YES, MayHaveConfig::YES, "<roughness>");
    this->DeclareOption(grp2, "metallic", "", "Metallic coefficient (0.0-1.0)", options.getAsDoubleRef("model.material.metallic"), HasDefault::YES, MayHaveConfig::YES, "<metallic>");
    this->DeclareOption(grp2, "hdri", "", "Path to an image file that will be used as a light source", options.getAsStringRef("render.background.hdri"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-matcap", "", "Path to a texture file containing a material capture", options.getAsStringRef("model.matcap.texture"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-base-color", "", "Path to a texture file that sets the color of the object", options.getAsStringRef("model.color.texture"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-material", "", "Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object", options.getAsStringRef("model.material.texture"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "texture-emissive", "", "Path to a texture file that sets the emitted light of the object", options.getAsStringRef("model.emissive.texture"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "emissive-factor", "", "Emissive factor. This value is multiplied with the emissive color when an emissive texture is present", options.getAsDoubleVectorRef("model.emissive.factor"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>");
    this->DeclareOption(grp2, "texture-normal", "", "Path to a texture file that sets the normal map of the object", options.getAsStringRef("model.normal.texture"), LocalHasDefaultNo, MayHaveConfig::YES, "<file path>");
    this->DeclareOption(grp2, "normal-scale", "", "Normal scale affects the strength of the normal deviation from the normal texture", options.getAsDoubleRef("model.normal.scale"), HasDefault::YES, MayHaveConfig::YES, "<normalScale>");

    auto grp3 = cxxOptions.add_options("Window");
    this->DeclareOption(grp3, "bg-color", "", "Background color", options.getAsDoubleVectorRef("render.background.color"), HasDefault::YES, MayHaveConfig::YES, "<R,G,B>");
    this->DeclareOption(grp3, "resolution", "", "Window resolution", appOptions.Resolution, HasDefault::YES, MayHaveConfig::YES, "<width,height>");
    this->DeclareOption(grp3, "position", "", "Window position", appOptions.Position, HasDefault::YES, MayHaveConfig::YES, "<x,y>");
    this->DeclareOption(grp3, "fps", "z", "Display frame per second", options.getAsBoolRef("ui.fps"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "filename", "n", "Display filename", options.getAsBoolRef("ui.filename"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "metadata", "m", "Display file metadata", options.getAsBoolRef("ui.metadata"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "blur-background", "u", "Blur background", options.getAsBoolRef("render.background.blur"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "blur-coc", "", "Blur circle of confusion radius", options.getAsDoubleRef("render.background.blur.coc"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp3, "light-intensity", "", "Light intensity", options.getAsDoubleRef("render.light.intensity"), HasDefault::YES, MayHaveConfig::YES, "<intensity>");

    auto grp4 = cxxOptions.add_options("Scientific visualization");
    this->DeclareOption(grp4, "scalars", "s", "Color by scalars", options.getAsStringRef("model.scivis.array-name"), HasDefault::YES, MayHaveConfig::YES, "<array_name>", HasImplicitValue::YES, "");
    this->DeclareOption(grp4, "comp", "y", "Component from the scalar array to color with. -1 means magnitude, -2 or the short option, -y, means direct scalars", options.getAsIntRef("model.scivis.component"), HasDefault::YES, MayHaveConfig::YES, "<comp_index>", HasImplicitValue::YES, "-2");
    this->DeclareOption(grp4, "cells", "c", "Use a scalar array from the cells", options.getAsBoolRef("model.scivis.cells"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "range", "", "Custom range for the coloring by array", options.getAsDoubleVectorRef("model.scivis.range"), HasDefault::YES, MayHaveConfig::YES, "<min,max>");
    this->DeclareOption(grp4, "bar", "b", "Show scalar bar", options.getAsBoolRef("ui.bar"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "colormap", "", "Specify a custom colormap", options.getAsDoubleVectorRef("model.scivis.colormap"), HasDefault::YES, MayHaveConfig::YES, "<color_list>");
    this->DeclareOption(grp4, "volume", "v", "Show volume if the file is compatible", options.getAsBoolRef("model.volume.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp4, "inverse", "i", "Inverse opacity function for volume rendering", options.getAsBoolRef("model.volume.inverse"), HasDefault::YES, MayHaveConfig::YES);

    auto grpCamera = cxxOptions.add_options("Camera");
    this->DeclareOption(grpCamera, "camera-position", "", "Camera position (overrides camera direction and camera zoom factor if any)", appOptions.CameraPosition, HasDefault::YES, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-focal-point", "", "Camera focal point", appOptions.CameraFocalPoint, HasDefault::YES, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-up", "", "Camera view up", appOptions.CameraViewUp, HasDefault::YES, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-view-angle", "", "Camera view angle (non-zero, in degrees)", appOptions.CameraViewAngle, LocalHasDefaultNo, MayHaveConfig::YES, "<angle>");
    this->DeclareOption(grpCamera, "camera-direction", "", "Camera direction", appOptions.CameraDirection, HasDefault::YES, MayHaveConfig::YES, "<X,Y,Z>");
    this->DeclareOption(grpCamera, "camera-zoom-factor", "", "Camera zoom factor (non-zero)", appOptions.CameraZoomFactor, HasDefault::YES, MayHaveConfig::YES, "<factor>");
    this->DeclareOption(grpCamera, "camera-azimuth-angle", "", "Camera azimuth angle (in degrees), performed after other camera options", appOptions.CameraAzimuthAngle, HasDefault::YES, MayHaveConfig::YES, "<angle>");
    this->DeclareOption(grpCamera, "camera-elevation-angle", "", "Camera elevation angle (in degrees), performed after other camera options", appOptions.CameraElevationAngle, HasDefault::YES, MayHaveConfig::YES, "<angle>");

#if F3D_MODULE_RAYTRACING
    auto grp5 = cxxOptions.add_options("Raytracing");
    this->DeclareOption(grp5, "raytracing", "r", "Enable raytracing", options.getAsBoolRef("render.raytracing.enable"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp5, "samples", "", "Number of samples per pixel", options.getAsIntRef("render.raytracing.samples"), HasDefault::YES, MayHaveConfig::YES, "<samples>");
    this->DeclareOption(grp5, "denoise", "d", "Denoise the image", options.getAsBoolRef("render.raytracing.denoise"), HasDefault::YES, MayHaveConfig::YES);
#endif

    auto grp6 = cxxOptions.add_options("PostFX (OpenGL)");
    this->DeclareOption(grp6, "translucency-support", "p", "Enable translucency support, implemented using depth peeling", options.getAsBoolRef("render.effect.translucency-support"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "ambient-occlusion", "q", "Enable ambient occlusion providing approximate shadows for better depth perception, implemented using SSAO", options.getAsBoolRef("render.effect.ambient-occlusion"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "anti-aliasing", "a", "Enable anti-aliasing, implemented using FXAA", options.getAsBoolRef("render.effect.anti-aliasing"), HasDefault::YES, MayHaveConfig::YES);
    this->DeclareOption(grp6, "tone-mapping", "t", "Enable Tone Mapping, providing balanced coloring", options.getAsBoolRef("render.effect.tone-mapping"), HasDefault::YES, MayHaveConfig::YES);

    auto grp7 = cxxOptions.add_options("Testing");
    this->DeclareOption(grp7, "ref", "", "Reference", appOptions.Reference, LocalHasDefaultNo, MayHaveConfig::YES, "<png file>");
    this->DeclareOption(grp7, "ref-threshold", "", "Testing threshold", appOptions.RefThreshold, HasDefault::YES, MayHaveConfig::YES, "<threshold>");
    this->DeclareOption(grp7, "interaction-test-record", "", "Path to an interaction log file to record interactions events to", appOptions.InteractionTestRecordFile, LocalHasDefaultNo, MayHaveConfig::YES, "<file_path>");
    this->DeclareOption(grp7, "interaction-test-play", "", "Path to an interaction log file to play interaction events from when loading a file", appOptions.InteractionTestPlayFile, LocalHasDefaultNo, MayHaveConfig::YES,"<file_path>");
    // clang-format on

    cxxOptions.positional_help("file1 file2 ...");
    cxxOptions.parse_positional({ "input" });
    cxxOptions.show_positional_help();
    cxxOptions.allow_unrecognised_options();

    if (parseCommandLine)
    {
      auto result = cxxOptions.parse(this->Argc, this->Argv);

      auto unmatched = result.unmatched();

      if (unmatched.size() > 0)
      {
        constexpr int distanceThreshold = 5;

        for (std::string unknownOption : unmatched)
        {
          // check if it's a long option
          if (unknownOption[0] == '-' && unknownOption[1] == '-')
          {
            // remove trailing '--'
            unknownOption = unknownOption.substr(2);

            auto it = std::find_if(unknownOption.rbegin(), unknownOption.rend(),
              [](unsigned char ch) { return ch == '='; });

            // remove everything after the character '='
            if (it != unknownOption.rend())
            {
              unknownOption.erase(it.base() - 1, unknownOption.end());
            }

            f3d::log::error("Unknown option --", unknownOption);

            auto [name, dist] = this->GetClosestOption(unknownOption);

            if (dist < distanceThreshold)
            {
              f3d::log::error("Did you mean --", name, "?");
            }
          }
        }
        throw F3DExNoProcess("unknown options");
      }

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

      if (result.count("scan-plugins") > 0)
      {
        this->PrintPluginsScan();
        throw F3DExNoProcess("scan plugins requested");
      }

      if (result.count("readers-list") > 0)
      {
        this->LoadPlugins(appOptions);
        this->PrintReadersList();
        throw F3DExNoProcess("reader list requested");
      }
    }
    else
    {
      // this will update the options using the config file without parsing actual argc/argv
      cxxOptions.parse(1, nullptr);
    }
  }
  catch (const cxxopts::exceptions::exception& ex)
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
void ConfigurationOptions::PrintHelp(const cxxopts::Options& cxxOptions)
{
  const std::vector<std::pair<std::string, std::string> > examples = {
    { this->ExecutableName + " file.vtu -xtgans",
      "View a unstructured mesh in a typical nice looking sciviz style" },
    { this->ExecutableName + " file.glb -tuqap --hdri=file.hdr",
      "View a gltf file in a realistic environment" },
    { this->ExecutableName + " file.ply -so --point-size=0 --comp=-2",
      "View a point cloud file with direct scalars rendering" },
    { this->ExecutableName + " folder", "View all files in folder" },
  };

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
  f3d::log::info("Version: " + libInfo.VersionFull + ".");
  f3d::log::info("Build date: " + libInfo.BuildDate + ".");
  f3d::log::info("Build system: " + libInfo.BuildSystem + ".");
  f3d::log::info("Compiler: " + libInfo.Compiler + ".");
  f3d::log::info("External rendering module: " + libInfo.ExternalRenderingModule + ".");
  f3d::log::info("Raytracing module: " + libInfo.RaytracingModule + ".");
  f3d::log::info("VTK version: " + libInfo.VTKVersion + ".");
  f3d::log::info(libInfo.PreviousCopyright + ".");
  f3d::log::info(libInfo.Copyright + ".");
  f3d::log::info("License " + libInfo.License + ".");
  f3d::log::info("By " + libInfo.Authors + ".");
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
  nameColSize++;
  extsColSize++;
  mimeColSize++;
  descColSize++;
  plugColSize++;

  std::string separator =
    std::string(nameColSize + extsColSize + descColSize + mimeColSize + plugColSize, '-');

  // Print the rows split in 3 columns
  std::stringstream headerLine;
  headerLine << std::left << std::setw(nameColSize) << "Name" << std::setw(plugColSize) << "Plugin"
             << std::setw(descColSize) << "Description" << std::setw(extsColSize) << "Exts"
             << std::setw(mimeColSize) << "Mime-types";
  f3d::log::info(headerLine.str());
  f3d::log::info(separator);

  for (const auto& reader : readersInfo)
  {
    for (size_t i = 0; i < reader.Extensions.size(); i++)
    {
      std::stringstream readerLine;
      if (i == 0)
      {
        readerLine << std::left << std::setw(nameColSize) << reader.Name << std::setw(plugColSize)
                   << reader.PluginName << std::setw(descColSize) << reader.Description;
      }
      else
      {
        readerLine << std::left << std::setw(nameColSize + descColSize + plugColSize) << " ";
      }

      readerLine << std::setw(extsColSize) << reader.Extensions[i];

      if (i < reader.MimeTypes.size())
      {
        readerLine << std::setw(mimeColSize) << reader.MimeTypes[i];
      }

      f3d::log::info(readerLine.str());
    }
    f3d::log::info(separator);
  }
  f3d::log::waitForUser();
}

//----------------------------------------------------------------------------
bool ConfigurationOptions::InitializeDictionaryFromConfigFile(const std::string& config)
{
  this->ConfigDic.clear();

  std::string configSearch = "config";
  if (!config.empty())
  {
    auto path = fs::path(config);
    if (path.stem() == config || path.filename() == config)
    {
      // Only a stem or a filename, use as search string
      configSearch = config;
    }
    else
    {
      // Assume its a full path and do not search for a config path
      configSearch = "";
    }
  }

  fs::path configPath;
  if (!configSearch.empty())
  {
    configPath = F3DConfigFileTools::GetConfigPath(configSearch);
  }
  else
  {
    configPath = config;
  }

  if (configPath.empty())
  {
    if (!config.empty())
    {
      f3d::log::error("Configuration file for \"", config, "\" could not been found");
    }
    return false;
  }

  // Recover an absolute canonical path to config file
  try
  {
    configPath = fs::canonical(fs::path(configPath)).string();
  }
  catch (const fs::filesystem_error&)
  {
    f3d::log::error("Configuration file does not exist: ", configPath.string());
    return false;
  }
  f3d::log::debug("Using config file ", configPath.string());

  // Recover all config files if needed
  std::set<fs::path> actualConfigFilePaths;
  if (fs::is_directory(configPath))
  {
    for (auto& entry : std::filesystem::directory_iterator(configPath))
    {
      actualConfigFilePaths.emplace(entry);
    }
  }
  else
  {
    actualConfigFilePaths.emplace(configPath);
  }

  // Read config files
  for (auto& configFilePath : actualConfigFilePaths)
  {
    std::ifstream file;
    file.open(configFilePath.string().c_str());

    if (!file.is_open())
    {
      f3d::log::error("Unable to open the configuration file: ", configFilePath.string());
      return false;
    }

    nlohmann::json json;
    try
    {
      file >> json;
    }
    catch (const std::exception& ex)
    {
      f3d::log::error("Unable to parse the configuration file ", configFilePath.string());
      f3d::log::error(ex.what());
      return false;
    }

    for (const auto& regexpConfig : json.items())
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
      if (regexpConfig.key() == "global")
      {
        this->GlobalConfigDicEntry = localDic;
      }
      else
      {
        this->ConfigDic[regexpConfig.key()] = localDic;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
F3DOptionsParser::F3DOptionsParser() = default;

//----------------------------------------------------------------------------
F3DOptionsParser::~F3DOptionsParser() = default;

//----------------------------------------------------------------------------
void F3DOptionsParser::Initialize(int argc, char** argv)
{
  this->ConfigOptions = std::make_unique<ConfigurationOptions>(argc, argv);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::InitializeDictionaryFromConfigFile(const std::string& userConfigFile)
{
  this->ConfigOptions->InitializeDictionaryFromConfigFile(userConfigFile);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::UpdateOptions(const std::string& filePath, F3DAppOptions& appOptions,
  f3d::options& options, bool parseCommandLine)
{
  std::vector<std::string> dummyFiles;
  return this->ConfigOptions->GetOptions(
    appOptions, options, dummyFiles, filePath, true, parseCommandLine);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::GetOptions(
  F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& files)
{
  return this->ConfigOptions->GetOptions(appOptions, options, files);
}

//----------------------------------------------------------------------------
void F3DOptionsParser::LoadPlugins(const F3DAppOptions& appOptions) const
{
  return this->ConfigOptions->LoadPlugins(appOptions);
}
