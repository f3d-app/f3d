#include "F3DStarter.h"

#include "F3DColorMapTools.h"
#include "F3DConfig.h"
#include "F3DConfigFileTools.h"
#include "F3DException.h"
#include "F3DIcon.h"
#include "F3DNSDelegate.h"
#include "F3DOptionsTools.h"
#include "F3DPluginsTools.h"
#include "F3DSystemTools.h"

#define DMON_IMPL
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4505)
#include "dmon.h"
// dmon includes Windows.h which defines 'ERROR' and conflicts with log.h
#undef ERROR
#pragma warning(pop)
#elif __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include "dmon.h"
#pragma clang diagnostic pop
#else
#include "dmon.h"
#endif

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "options.h"
#include "window.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <set>

namespace fs = std::filesystem;

class F3DStarter::F3DInternals
{
public:
  F3DInternals() = default;

  // XXX: The values in the following two structs
  // are left uninitialized as the will all be initialized from
  // F3DOptionsTools::DefaultAppOptions
  struct CameraConfiguration
  {
    std::vector<double> CameraPosition;
    std::vector<double> CameraFocalPoint;
    std::vector<double> CameraViewUp;
    double CameraViewAngle;
    std::vector<double> CameraDirection;
    double CameraZoomFactor;
    double CameraAzimuthAngle;
    double CameraElevationAngle;
  };

  struct F3DAppOptions
  {
    std::string Output;
    bool BindingsList;
    bool NoBackground;
    bool NoRender;
    std::string RenderingBackend;
    double MaxSize;
    bool Watch;
    std::vector<std::string> Plugins;
    std::string ScreenshotFilename;
    std::string VerboseLevel;
    std::string MultiFileMode;
    std::vector<int> Resolution;
    std::vector<int> Position;
    std::string ColorMapFile;
    CameraConfiguration CamConf;
    std::string Reference;
    double RefThreshold;
    std::string InteractionTestRecordFile;
    std::string InteractionTestPlayFile;
  };

  void SetupCamera(const CameraConfiguration& camConf)
  {
    f3d::camera& cam = this->Engine->getWindow().getCamera();
    if (camConf.CameraPosition.size() == 3)
    {
      f3d::point3_t pos;
      std::copy_n(camConf.CameraPosition.begin(), 3, pos.begin());
      cam.setPosition(pos);
    }
    if (camConf.CameraFocalPoint.size() == 3)
    {
      f3d::point3_t foc;
      std::copy_n(camConf.CameraFocalPoint.begin(), 3, foc.begin());
      cam.setFocalPoint(foc);
    }
    if (camConf.CameraViewUp.size() == 3)
    {
      f3d::vector3_t up;
      std::copy_n(camConf.CameraViewUp.begin(), 3, up.begin());
      cam.setViewUp(up);
    }
    if (camConf.CameraViewAngle > 0)
    {
      cam.setViewAngle(camConf.CameraViewAngle);
    }

    bool reset = false;
    double zoomFactor = 0.9;
    if (camConf.CameraPosition.size() != 3 && camConf.CameraDirection.size() == 3)
    {
      f3d::vector3_t dir;
      std::copy_n(camConf.CameraDirection.begin(), 3, dir.begin());
      f3d::point3_t foc;
      f3d::point3_t pos;
      cam.getFocalPoint(foc);
      for (int i = 0; i < 3; i++)
      {
        pos[i] = foc[i] - dir[i];
      }
      cam.setPosition(pos);
      reset = true;
    }
    if (camConf.CameraPosition.size() != 3 && camConf.CameraZoomFactor > 0)
    {
      zoomFactor = camConf.CameraZoomFactor;
      reset = true;
    }
    if (reset)
    {
      cam.resetToBounds(zoomFactor);
    }

    cam.azimuth(camConf.CameraAzimuthAngle)
      .elevation(camConf.CameraElevationAngle)
      .setCurrentAsDefault();
  }

  static bool HasHDRIExtension(const std::string& file)
  {
    std::string ext = fs::path(file).extension().string();
    for (char& c : ext)
    {
      // casting is required on Windows
      c = static_cast<char>(toupper(c));
    }
#if F3D_MODULE_EXR
    if (ext == ".EXR")
    {
      return true;
    }
#endif
    if (ext == ".HDR")
    {
      return true;
    }
    return false;
  }

  static void SetVerboseLevel(const std::string& level, bool forceStdErr)
  {
    // A switch/case over verbose level
    if (level == "quiet")
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::QUIET, forceStdErr);
    }
    else if (level == "error")
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::ERROR, forceStdErr);
    }
    else if (level == "warning")
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::WARN, forceStdErr);
    }
    else if (level == "info")
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO, forceStdErr);
    }
    else if (level == "debug")
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG, forceStdErr);
    }
    else
    {
      f3d::log::warn("Unrecognized verbose level: ", level,
        ", Ignoring. Possible values are quiet, error, warning, info, debug");
    }

    if (forceStdErr)
    {
      f3d::log::info("Output image will be saved to stdout, all log types including debug and info "
                     "levels are redirected to stderr");
    }
  }

  static void dmonFolderChanged(
    dmon_watch_id, dmon_action, const char*, const char* filename, const char*, void* userData)
  {
    F3DStarter* self = reinterpret_cast<F3DStarter*>(userData);
    const std::lock_guard<std::mutex> lock(self->Internals->LoadedFilesMutex);
    if (std::find_if(self->Internals->LoadedFiles.begin(), self->Internals->LoadedFiles.end(),
          [&](const auto& path)
          { return path.filename() == filename; }) != self->Internals->LoadedFiles.end())
    {
      self->Internals->ReloadFileRequested = true;
    }
  }

  void addOutputImageMetadata(f3d::image& image)
  {
    std::stringstream cameraMetadata;
    {
      const auto state = Engine->getWindow().getCamera().getState();
      const auto vec3toJson = [](const std::array<double, 3>& v)
      {
        std::stringstream ss;
        ss << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
        return ss.str();
      };
      cameraMetadata << "{\n";
      cameraMetadata << "  \"pos\": " << vec3toJson(state.pos) << ",\n";
      cameraMetadata << "  \"foc\": " << vec3toJson(state.foc) << ",\n";
      cameraMetadata << "  \"up\": " << vec3toJson(state.up) << ",\n";
      cameraMetadata << "  \"angle\": " << state.angle << "\n";
      cameraMetadata << "}\n";
    }

    image.setMetadata("camera", cameraMetadata.str());
  }

  /**
   * Substitute the following variables in a filename template:
   * - `{app}`: application name (ie. `F3D`)
   * - `{version}`: application version (eg. `2.4.0`)
   * - `{version_full}`: full application version (eg. `2.4.0-abcdefgh`)
   * - `{model}`: current model filename without extension (eg. `foo` for `/home/user/foo.glb`)
   * - `{model.ext}`: current model filename with extension (eg. `foo.glb` for `/home/user/foo.glb`)
   * - `{model_ext}`: current model filename extension (eg. `glb` for `/home/user/foo.glb`)
   * - `{date}`: current date in YYYYMMDD format
   * - `{date:format}`: current date as per C++'s `std::put_time` format
   * - `{n}`: auto-incremented number to make filename unique (up to 1000000)
   * - `{n:2}`, `{n:3}`, ...: zero-padded auto-incremented number to make filename unique
   *   (up to 1000000)
   */
  fs::path applyFilenameTemplate(const std::string& templateString)
  {
    constexpr size_t maxNumberingAttempts = 1000000;
    const std::regex numberingRe("\\{(n:?([0-9]*))\\}");
    const std::regex dateRe("date:?([A-Za-z%]*)");

    /* Return a file related string depending on the currently loaded files, or the empty string if
     * a single file is loaded */
    const auto fileCheck = [&]()
    {
      if (this->LoadedFiles.empty())
      {
        return "no_file";
      }
      else if (this->LoadedFiles.size() > 1)
      {
        return "multi_file";
      }
      else
      {
        return "";
      }
    };

    /* return value for template variable name (eg. `app` -> `F3D`) */
    const auto variableLookup = [&](const std::string& var)
    {
      if (var == "app")
      {
        return F3D::AppName;
      }
      else if (var == "version")
      {
        return F3D::AppVersion;
      }
      else if (var == "version_full")
      {
        return F3D::AppVersionFull;
      }
      else if (var == "model")
      {
        std::string output = fileCheck();
        if (output.empty())
        {
          output = this->LoadedFiles[0].stem().string();
        }
        return output;
      }
      else if (var == "model.ext")
      {
        std::string output = fileCheck();
        if (output.empty())
        {
          output = this->LoadedFiles[0].filename().string();
        }
        return output;
      }
      else if (var == "model_ext")
      {
        std::string output = fileCheck();
        if (output.empty())
        {
          output = this->LoadedFiles[0].extension().string().substr(1);
        }
        return output;
      }
      else if (std::regex_match(var, dateRe))
      {
        auto fmt = std::regex_replace(var, dateRe, "$1");
        if (fmt.empty())
        {
          fmt = "%Y%m%d";
        }
        std::time_t t = std::time(nullptr);
        std::stringstream joined;
        joined << std::put_time(std::localtime(&t), fmt.c_str());
        return joined.str();
      }
      throw std::out_of_range(var);
    };

    /* process template as tokens, keeping track of whether they've been
     * substituted or left untouched */
    const auto substituteVariables = [&]()
    {
      const std::string varName = "[\\w_.%:-]+";
      const std::string escapedVar = "(\\{(\\{" + varName + "\\})\\})";
      const std::string substVar = "(\\{(" + varName + ")\\})";
      const std::regex escapedVarRe(escapedVar);
      const std::regex substVarRe(substVar);

      std::vector<std::pair<std::string, bool>> fragments;
      const auto callback = [&](const std::string& m)
      {
        if (std::regex_match(m, escapedVarRe))
        {
          fragments.emplace_back(std::regex_replace(m, escapedVarRe, "$2"), true);
        }
        else if (std::regex_match(m, substVarRe))
        {
          try
          {
            fragments.emplace_back(variableLookup(std::regex_replace(m, substVarRe, "$2")), true);
          }
          catch (std::out_of_range&)
          {
            fragments.emplace_back(m, false);
          }
        }
        else
        {
          fragments.emplace_back(m, false);
        }
      };

      const std::regex re(escapedVar + "|" + substVar);
      std::sregex_token_iterator begin(templateString.begin(), templateString.end(), re, { -1, 0 });
      std::for_each(begin, std::sregex_token_iterator(), callback);

      return fragments;
    };

    const auto fragments = substituteVariables();

    /* check the non-substituted fragments for numbering variables */
    const auto hasNumbering = [&]()
    {
      for (const auto& [fragment, processed] : fragments)
      {
        if (!processed && std::regex_search(fragment, numberingRe))
        {
          return true;
        }
      }
      return false;
    };

    /* just join and return if there's no numbering to be done */
    if (!hasNumbering())
    {
      std::stringstream joined;
      for (const auto& fragment : fragments)
      {
        joined << fragment.first;
      }
      return { joined.str() };
    }

    /* apply numbering in the non-substituted fragments and join */
    const auto applyNumbering = [&](const size_t i)
    {
      std::stringstream joined;
      for (const auto& [fragment, processed] : fragments)
      {
        if (!processed && std::regex_match(fragment, numberingRe))
        {
          std::stringstream formattedNumber;
          try
          {
            const std::string fmt = std::regex_replace(fragment, numberingRe, "$2");
            formattedNumber << std::setfill('0') << std::setw(std::stoi(fmt)) << i;
          }
          catch (std::invalid_argument&)
          {
            formattedNumber << std::setw(0) << i;
          }
          joined << std::regex_replace(fragment, numberingRe, formattedNumber.str());
        }
        else
        {
          joined << fragment;
        }
      }
      return joined.str();
    };

    /* apply incrementing numbering until file doesn't exist already */
    for (size_t i = 1; i <= maxNumberingAttempts; ++i)
    {
      const std::string candidate = applyNumbering(i);
      if (!fs::exists(candidate))
      {
        return { candidate };
      }
    }
    throw std::runtime_error("could not find available unique filename after " +
      std::to_string(maxNumberingAttempts) + " attempts");
  }

  void UpdateOptions(const std::vector<F3DOptionsTools::OptionsEntries>& entriesVector,
    const std::vector<fs::path>& paths)
  {
    assert(!paths.empty());

    f3d::log::debug("Updating Options:");
    // Initialize libf3dOptions
    f3d::options libOptions;
    libOptions.ui.dropzone_info = "Drop a file or HDRI to load it\nPress H to show cheatsheet";

    // Copy appOptions
    F3DOptionsTools::OptionsDict appOptions = F3DOptionsTools::DefaultAppOptions;

    // Logging specific map
    bool logOptions = this->AppOptions.VerboseLevel == "debug";
    std::map<std::string, std::tuple<std::string, fs::path, std::string, std::string>> loggingMap;

    // For each input file, order matter
    for (const auto& tmpPath : paths)
    {
      std::string inputFile = tmpPath.string();
      // For each config entries, ordered by priority
      for (const auto& entries : entriesVector)
      {
        // For each entry (eg: difference config files)
        for (auto const& [conf, source, pattern] : entries)
        {
          std::regex re(pattern, std::regex_constants::icase);
          std::smatch matches;
          // If the source is empty, there is no pattern, all options applies
          // Note: An empty inputFile matches with ".*"
          if (source.empty() || std::regex_match(inputFile, matches, re))
          {
            // For each option key/value
            for (auto const& [key, value] : conf)
            {
              // Check in appOptions first
              auto appIter = appOptions.find(key);
              if (appIter != appOptions.end())
              {
                appOptions[key] = value;
                if (logOptions)
                {
                  loggingMap.emplace(key, std::tuple(key, source, pattern, value));
                }
                continue;
              }

              std::string libf3dOptionName = key;

              // Convert key into a libf3d option name if possible
              auto libf3dIter = F3DOptionsTools::LibOptionsNames.find(key);
              if (libf3dIter != F3DOptionsTools::LibOptionsNames.end())
              {
                libf3dOptionName = std::string(libf3dIter->second);
              }

              try
              {
                // Assume this is a libf3d option and set the value
                libOptions.setAsString(libf3dOptionName, value);

                // Log the option if needed
                if (logOptions)
                {
                  loggingMap.emplace(libf3dOptionName, std::tuple(key, source, pattern, value));
                }
              }
              catch (const f3d::options::parsing_exception& ex)
              {
                std::string origin =
                  source.empty() ? pattern : source.string() + ":`" + pattern + "`";
                f3d::log::warn("Could not set '", key, "' to '", value, "' from ", origin,
                  " because: ", ex.what());
              }
              catch (const f3d::options::inexistent_exception&)
              {
                std::string origin =
                  source.empty() ? pattern : source.string() + ":`" + pattern + "`";
                auto [closestName, dist] =
                  F3DOptionsTools::GetClosestOption(libf3dOptionName, true);
                f3d::log::warn("'", key, "' option from ", origin,
                  " does not exists , did you mean '", closestName, "'?");
              }
            }
          }
        }
      }
    }

    for (const auto& [key, tuple] : loggingMap)
    {
      const auto& [name, source, pattern, value] = tuple;
      std::string origin = source.empty() ? pattern : source.string() + ":`" + pattern + "`";
      f3d::log::debug(" '", name, "' = '", value, "' from ", origin);
    }
    f3d::log::debug("");

    // Update typed app options from the string version
    this->UpdateTypedAppOptions(appOptions);

    // Update Verbose level as soon as possible
    F3DInternals::SetVerboseLevel(this->AppOptions.VerboseLevel, this->AppOptions.Output == "-");

    // Load any new plugins
    F3DPluginsTools::LoadPlugins(this->AppOptions.Plugins);

    // Update libf3d options
    this->LibOptions = libOptions;

    // Update options that depends on both app and libf3d options
    this->UpdateInterdependentOptions();
  }

  void UpdateTypedAppOptions(const std::map<std::string, std::string>& appOptions)
  {
    // Update typed app options from app options
    this->AppOptions.Output = f3d::options::parse<std::string>(appOptions.at("output"));
    this->AppOptions.BindingsList = f3d::options::parse<bool>(appOptions.at("bindings-list"));
    this->AppOptions.NoBackground = f3d::options::parse<bool>(appOptions.at("no-background"));
    this->AppOptions.NoRender = f3d::options::parse<bool>(appOptions.at("no-render"));
    this->AppOptions.RenderingBackend =
      f3d::options::parse<std::string>(appOptions.at("rendering-backend"));
    this->AppOptions.MaxSize = f3d::options::parse<double>(appOptions.at("max-size"));
    this->AppOptions.Watch = f3d::options::parse<bool>(appOptions.at("watch"));
    this->AppOptions.Plugins = { f3d::options::parse<std::vector<std::string>>(
      appOptions.at("load-plugins")) };
    this->AppOptions.ScreenshotFilename =
      f3d::options::parse<std::string>(appOptions.at("screenshot-filename"));
    this->AppOptions.VerboseLevel = f3d::options::parse<std::string>(appOptions.at("verbose"));
    this->AppOptions.MultiFileMode =
      f3d::options::parse<std::string>(appOptions.at("multi-file-mode"));
    this->AppOptions.Resolution =
      f3d::options::parse<std::vector<int>>(appOptions.at("resolution"));
    this->AppOptions.Position = f3d::options::parse<std::vector<int>>(appOptions.at("position"));
    this->AppOptions.ColorMapFile =
      f3d::options::parse<std::string>(appOptions.at("colormap-file"));

    this->AppOptions.CamConf = { f3d::options::parse<std::vector<double>>(
                                   appOptions.at("camera-position")),
      f3d::options::parse<std::vector<double>>(appOptions.at("camera-focal-point")),
      f3d::options::parse<std::vector<double>>(appOptions.at("camera-view-up")),
      f3d::options::parse<double>(appOptions.at("camera-view-angle")),
      f3d::options::parse<std::vector<double>>(appOptions.at("camera-direction")),
      f3d::options::parse<double>(appOptions.at("camera-zoom-factor")),
      f3d::options::parse<double>(appOptions.at("camera-azimuth-angle")),
      f3d::options::parse<double>(appOptions.at("camera-elevation-angle")) };

    this->AppOptions.Reference = f3d::options::parse<std::string>(appOptions.at("ref"));
    this->AppOptions.RefThreshold = f3d::options::parse<double>(appOptions.at("ref-threshold"));
    this->AppOptions.InteractionTestRecordFile =
      f3d::options::parse<std::string>(appOptions.at("interaction-test-record"));
    this->AppOptions.InteractionTestPlayFile =
      f3d::options::parse<std::string>(appOptions.at("interaction-test-play"));
  }

  void UpdateInterdependentOptions()
  {
    // colormap-file and colormap are interdependent
    const std::string& colorMapFile = this->AppOptions.ColorMapFile;
    if (!colorMapFile.empty())
    {
      std::string fullPath = F3DColorMapTools::Find(colorMapFile);

      if (!fullPath.empty())
      {
        this->LibOptions.model.scivis.colormap = F3DColorMapTools::Read(fullPath);
      }
      else
      {
        f3d::log::error("Cannot find the colormap ", colorMapFile);
        this->LibOptions.model.scivis.colormap = std::vector<double>{};
      }
    }
  }

  void ApplyPositionAndResolution()
  {
    if (!this->AppOptions.NoRender)
    {
      f3d::window& window = this->Engine->getWindow();
      if (this->AppOptions.Resolution.size() == 2)
      {
        window.setSize(this->AppOptions.Resolution[0], this->AppOptions.Resolution[1]);
      }
      else if (!this->AppOptions.Resolution.empty())
      {
        f3d::log::warn("Provided resolution could not be applied");
      }

      if (this->AppOptions.Position.size() == 2)
      {
        window.setPosition(this->AppOptions.Position[0], this->AppOptions.Position[1]);
      }
      else
      {
        if (!this->AppOptions.Position.empty())
        {
          f3d::log::warn("Provided position could not be applied");
        }

#ifdef __APPLE__
        // The default position (50, 50) in VTK on MacOS is not a great fit for F3D as it can be
        // partially hidden because the position correspond to the upper left corner and the Y
        // position is defined from the bottom of the screen. Position it somewhere it makes sense.
        window.setPosition(100, 800);
#endif
      }
    }
  }

  // Recover a vector of unique parent paths from a vector of paths
  static std::vector<fs::path> ParentPaths(const std::vector<fs::path>& paths)
  {
    std::vector<fs::path> parents;
    for (const auto& tmpPath : paths)
    {
      fs::path parentPath = tmpPath.parent_path();
      if (std::find(parents.begin(), parents.end(), tmpPath) == parents.end())
      {
        parents.emplace_back(parentPath);
      }
    }
    return parents;
  }

  F3DAppOptions AppOptions;
  f3d::options LibOptions;
  F3DOptionsTools::OptionsEntries ConfigOptionsEntries;
  F3DOptionsTools::OptionsEntries CLIOptionsEntries;
  F3DOptionsTools::OptionsEntries DynamicOptionsEntries;
  std::unique_ptr<f3d::engine> Engine;
  std::vector<std::vector<fs::path>> FilesGroups;
  std::vector<fs::path> LoadedFiles;
  std::vector<dmon_watch_id> FolderWatchIds;
  int CurrentFilesGroupIndex = -1;

  // dmon used atomic and mutex
  std::mutex LoadedFilesMutex;

  // Event loop atomics
  std::atomic<bool> RenderRequested = false;
  std::atomic<bool> ReloadFileRequested = false;
};

//----------------------------------------------------------------------------
F3DStarter::F3DStarter()
  : Internals(std::make_unique<F3DStarter::F3DInternals>())
{
  // Initialize dmon
  dmon_init();
}

//----------------------------------------------------------------------------
F3DStarter::~F3DStarter()
{
  // deinit dmon
  dmon_deinit();
}

//----------------------------------------------------------------------------
int F3DStarter::Start(int argc, char** argv)
{
  // Parse CLI Options into an option dict
  std::vector<std::string> inputFiles;
  F3DOptionsTools::OptionsDict cliOptionsDict =
    F3DOptionsTools::ParseCLIOptions(argc, argv, inputFiles);

  // Store in a option entries for easier processing
  this->Internals->CLIOptionsEntries.emplace_back(cliOptionsDict, fs::path(), "CLI options");

  // Check dry-run, config CLI, output and verbose options first
  // XXX: the local variable are initialized manually for simplicity
  // but this duplicate the initialization value as it is present in
  // F3DOptionTools::DefaultAppOptions too
  bool dryRun = false;
  if (cliOptionsDict.find("no-render") != cliOptionsDict.end())
  {
    dryRun = f3d::options::parse<bool>(cliOptionsDict["no-render"]);
  }
  std::string config;
  if (cliOptionsDict.find("config") != cliOptionsDict.end())
  {
    config = f3d::options::parse<std::string>(cliOptionsDict["config"]);
  }
  bool renderToStdout = false;
  if (cliOptionsDict.find("output") != cliOptionsDict.end())
  {
    renderToStdout = f3d::options::parse<std::string>(cliOptionsDict["output"]) == "-";
  }
  this->Internals->AppOptions.VerboseLevel = "info";
  if (cliOptionsDict.find("verbose") != cliOptionsDict.end())
  {
    this->Internals->AppOptions.VerboseLevel =
      f3d::options::parse<std::string>(cliOptionsDict["verbose"]);
  }

  // Set verbosity level early from command line
  F3DInternals::SetVerboseLevel(this->Internals->AppOptions.VerboseLevel, renderToStdout);

  f3d::log::debug("========== Initializing Options ==========");

  // Read config files
  if (!dryRun)
  {
    this->Internals->ConfigOptionsEntries = F3DConfigFileTools::ReadConfigFiles(config);
  }

  // Update app and libf3d options based on config entries, with an empty input file
  // config < cli
  this->Internals->UpdateOptions(
    { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries }, { "" });

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  f3d::log::debug("========== Configuring engine ==========");

  const std::string& reference = this->Internals->AppOptions.Reference;
  const std::string& output = this->Internals->AppOptions.Output;

  if (this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createNone());
  }
  else
  {
    bool offscreen =
      !reference.empty() || !output.empty() || this->Internals->AppOptions.BindingsList;

    if (this->Internals->AppOptions.RenderingBackend == "egl")
    {
      this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createEGL());
    }
    else if (this->Internals->AppOptions.RenderingBackend == "osmesa")
    {
      this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createOSMesa());
    }
    else if (this->Internals->AppOptions.RenderingBackend == "glx")
    {
      this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createGLX(offscreen));
    }
    else if (this->Internals->AppOptions.RenderingBackend == "wgl")
    {
      this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createWGL(offscreen));
    }
    else
    {
      if (this->Internals->AppOptions.RenderingBackend != "auto")
      {
        f3d::log::warn("--rendering-backend value is invalid, falling back to \"auto\"");
      }
      this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::create(offscreen));
    }

    f3d::window& window = this->Internals->Engine->getWindow();
    window.setWindowName(F3D::AppTitle).setIcon(F3DIcon, sizeof(F3DIcon));
    this->Internals->ApplyPositionAndResolution();

    f3d::interactor& interactor = this->Internals->Engine->getInteractor();

    interactor.addCommand("load_previous_file_group",
      [this](const std::vector<std::string>&) { this->LoadRelativeFileGroup(-1); });

    interactor.addCommand("load_next_file_group",
      [this](const std::vector<std::string>&) { this->LoadRelativeFileGroup(+1); });

    interactor.addCommand("reload_current_file_group",
      [this](const std::vector<std::string>&) { this->LoadRelativeFileGroup(0, true, true); });

    interactor.addCommand("add_current_directories",
      [this](const std::vector<std::string>&)
      {
        if (this->Internals->LoadedFiles.size() > 0)
        {
          for (const auto& parentPath : F3DInternals::ParentPaths(this->Internals->LoadedFiles))
          {
            this->AddFile(parentPath, true);
          }
          this->LoadRelativeFileGroup(0);
        }
      });

    interactor.addCommand("take_screenshot",
      [this](const std::vector<std::string>& args)
      {
        // XXX: Add a test for this one this can be reached with a non empty filename
        std::string filename =
          args.empty() ? this->Internals->AppOptions.ScreenshotFilename : args[0];
        this->SaveScreenshot(filename);
      });

    interactor.addCommand("take_minimal_screenshot",
      [this](const std::vector<std::string>& args)
      {
        // XXX: Add a test for this one this can be reached with a non empty filename
        std::string filename =
          args.empty() ? this->Internals->AppOptions.ScreenshotFilename : args[0];
        this->SaveScreenshot(filename, true);
      });

    // This replace an existing command in libf3d
    interactor.removeCommand("add_files");
    interactor.addCommand("add_files",
      [this](const std::vector<std::string>& files)
      {
        int index = -1;
        for (const std::string& file : files)
        {
          index = this->AddFile(fs::path(file));
        }
        if (index > -1)
        {
          this->LoadFileGroup(index);
        }
      });

    interactor.addCommand("set_hdri",
      [this](const std::vector<std::string>& files)
      {
        if (!files.empty())
        {
          // Set the first file has an HDRI
          f3d::options& options = this->Internals->Engine->getOptions();
          options.render.hdri.file = files[0];
          options.render.hdri.ambient = true;
          options.render.background.skybox = true;

          // Rendering now is needed for correct lighting
          this->Render();
        }
      });

    interactor.addCommand("add_files_or_set_hdri",
      [this](const std::vector<std::string>& files)
      {
        int index = -1;
        for (const std::string& file : files)
        {
          if (F3DInternals::HasHDRIExtension(file))
          {
            // TODO: add a image::canRead

            // Load the file as an HDRI instead of adding it.
            f3d::options& options = this->Internals->Engine->getOptions();
            options.render.hdri.file = file;
            options.render.hdri.ambient = true;
            options.render.background.skybox = true;

            // Rendering now is needed for correct lighting
            this->Render();
          }
          else
          {
            index = this->AddFile(fs::path(file));
          }
        }
        if (index > -1)
        {
          this->LoadFileGroup(index);
        }
      });

    // "doc"
    auto docString = [](const std::string& doc) -> std::pair<std::string, std::string>
    { return std::make_pair(doc, ""); };

    using mod_t = f3d::interaction_bind_t::ModifierKeys;
    interactor.addBinding({mod_t::NONE, "Left"}, "load_previous_file_group", "Others",
      std::bind(docString, "Load previous file group"));
    interactor.addBinding({mod_t::NONE, "Right"}, "load_next_file_group", "Others",
      std::bind(docString, "Load next file group"));
    interactor.addBinding({mod_t::NONE, "Up"}, "reload_current_file_group", "Others",
      std::bind(docString, "Reload current file group"));
    interactor.addBinding({mod_t::NONE, "Down"}, "add_current_directories", "Others",
      std::bind(docString, "Add files from dir of current file"));
    interactor.addBinding({mod_t::NONE, "F11"}, "take_minimal_screenshot", "Others",
      std::bind(docString, "Take a minimal screenshot"));
    interactor.addBinding({mod_t::NONE, "F12"}, "take_screenshot", "Others",
      std::bind(docString, "Take a screenshot"));

    // This replace an existing default interaction command in the libf3d
    interactor.removeBinding({mod_t::NONE, "Drop"});
    interactor.addBinding({mod_t::NONE, "Drop"}, "add_files_or_set_hdri", "Others",
      std::bind(docString, "Load dropped files, folder or HDRI"));
    interactor.addBinding({mod_t::CTRL, "Drop"}, "add_files", "Others",
      std::bind(docString, "Load dropped files or folder"));
    interactor.addBinding({mod_t::SHIFT, "Drop"}, "set_hdri", "Others",
      std::bind(docString, "Set HDRI and use it"));
  }

  this->Internals->Engine->setOptions(this->Internals->LibOptions);
  f3d::log::debug("Engine configured");

  f3d::interactor& interactor = this->Internals->Engine->getInteractor();

  // Print bindings list and exits if needed
  if (this->Internals->AppOptions.BindingsList)
  {
    f3d::log::info("Bindings:");
    for (std::string group : interactor.getBindGroups())
    {
      f3d::log::info(" ", group, ":");
      for (const f3d::interaction_bind_t& bind : interactor.getBindsForGroup(group))
      {
        // XXX: Formating could be improved here
        auto [doc, val] = interactor.getBindingDocumentation(bind);
        F3DOptionsTools::PrintHelpPair(bind.format(), doc, 12);
      }
      f3d::log::info("");
    }
    throw F3DExNoProcess("bindings list requested");
  }

  // Add all input files
  for (auto& file : inputFiles)
  {
    this->AddFile(fs::path(file));
  }

  // Load a file
  this->LoadFileGroup();

  if (!this->Internals->AppOptions.NoRender)
  {
    f3d::window& window = this->Internals->Engine->getWindow();

    // Play recording if any
    const std::string& interactionTestPlayFile =
      this->Internals->AppOptions.InteractionTestPlayFile;
    if (!interactionTestPlayFile.empty())
    {
      // For better testing, render once before the interaction
      window.render();
      if (!interactor.playInteraction(interactionTestPlayFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Start recording if needed
    const std::string& interactionTestRecordFile =
      this->Internals->AppOptions.InteractionTestRecordFile;
    if (!interactionTestRecordFile.empty())
    {
      if (!interactor.recordInteraction(interactionTestRecordFile))
      {
        return EXIT_FAILURE;
      }
    }

    char* noDataForceRender = std::getenv("CTEST_F3D_NO_DATA_FORCE_RENDER");

    // Render and compare with file if needed
    if (!reference.empty())
    {
      if (this->Internals->LoadedFiles.empty() && !noDataForceRender)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!fs::exists(reference))
      {
        if (output.empty())
        {
          f3d::log::error("Reference image ", reference,
            " does not exist, use the output option to output current rendering into an image "
            "file.\n");
        }
        else
        {
          window.renderToImage(this->Internals->AppOptions.NoBackground).save(output);

          f3d::log::error("Reference image " + reference +
            " does not exist, current rendering has been outputted to " + output + ".\n");
        }
        return EXIT_FAILURE;
      }

      f3d::image img = window.renderToImage(this->Internals->AppOptions.NoBackground);
      f3d::image ref(reference);
      f3d::image diff;
      double error;
      const double& threshold = this->Internals->AppOptions.RefThreshold;
      if (!img.compare(ref, threshold, error))
      {
        if (output.empty())
        {
          f3d::log::error("Use the --output option to be able to output current rendering and diff "
                          "images into files.\n");
        }
        else
        {
          f3d::log::error("Current rendering difference with reference image: ", error,
            " is higher than the threshold of ", threshold, ".\n");

          img.save(output);
        }
        return EXIT_FAILURE;
      }
      else
      {
        f3d::log::info("Image comparison success with an error difference of: ", error);
      }

      if (this->Internals->FilesGroups.size() > 1)
      {
        f3d::log::warn("Image comparison was performed using a single 3D file, other provided "
                       "3D files were ignored.");
      }
    }
    // Render to file if needed
    else if (!output.empty())
    {
      if (this->Internals->LoadedFiles.empty() && !noDataForceRender)
      {
        f3d::log::error("No files loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      f3d::image img = window.renderToImage(this->Internals->AppOptions.NoBackground);
      this->Internals->addOutputImageMetadata(img);

      if (renderToStdout)
      {
        const auto buffer = img.saveBuffer();
        std::copy(buffer.begin(), buffer.end(), std::ostreambuf_iterator(std::cout));
        f3d::log::debug("Output image saved to stdout");
      }
      else
      {
        fs::path path = this->Internals->applyFilenameTemplate(output);
        img.save(path.string());
        f3d::log::debug("Output image saved to ", path);
      }

      if (this->Internals->FilesGroups.size() > 1)
      {
        f3d::log::warn("An output image was saved using a single 3D file, other provided 3D "
                       "files were ignored.");
      }
    }
    // Start interaction
    else
    {
#ifdef F3D_HEADLESS_BUILD
      f3d::log::error("This is a headless build of F3D, interactive rendering is not supported");
      return EXIT_FAILURE;
#else
      if (this->Internals->Engine->getWindow().isOffscreen())
      {
        f3d::log::warn(
          "You are using an offscreen configuration, interactive rendering is disabled");
        return EXIT_SUCCESS;
      }
      else
      {
        // Create the event loop repeating timer
        interactor.createTimerCallBack(30, [this]() { this->EventLoop(); });
        this->RequestRender();
        interactor.start();
      }
#endif
    }
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void F3DStarter::LoadFileGroup(int index, bool relativeIndex, bool forceClear)
{
  int groupIndex = this->Internals->CurrentFilesGroupIndex;
  if (relativeIndex)
  {
    groupIndex += index;
  }
  else
  {
    groupIndex = index;
  }

  // Compute a modulo to ensure 0 < groupIndex < size
  // XXX Do not work if groupIndex + size < 0
  int size = static_cast<int>(this->Internals->FilesGroups.size());
  if (size != 0)
  {
    groupIndex = (groupIndex + size) % size;
  }
  else
  {
    groupIndex = -1;
  }

  if (groupIndex >= 0)
  {
    // Clear only if we change the group to load
    bool clear = forceClear ? true : this->Internals->CurrentFilesGroupIndex != groupIndex;
    this->Internals->CurrentFilesGroupIndex = groupIndex;

    // Create a nice looking group index eg: "(1/5)"
    // XXX: Each group contains at least one path
    std::string groupIdx = "(" + std::to_string(groupIndex + 1) + "/" +
      std::to_string(this->Internals->FilesGroups.size()) + ")";
    this->LoadFileGroup(this->Internals->FilesGroups[groupIndex], clear, groupIdx);
  }
  else
  {
    this->Internals->CurrentFilesGroupIndex = groupIndex;
    this->LoadFileGroup(std::vector<fs::path>{}, true, "");
  }
}

//----------------------------------------------------------------------------
void F3DStarter::LoadFileGroup(
  const std::vector<fs::path>& paths, bool clear, const std::string& groupIdx)
{
  // Make sure the animation is stopped before trying to load any file
  if (!this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine->getInteractor().stopAnimation();
  }

  f3d::log::debug("========== Loading 3D files ==========");

  // Recover current options from the engine
  const f3d::options& dynamicOptions = this->Internals->Engine->getOptions();

  // Detect interactively changed options and store them into the dynamic options dict
  // options names are shared between options instance
  F3DOptionsTools::OptionsDict dynamicOptionsDict;
  std::vector<std::string> optionNames = dynamicOptions.getNames();
  for (const auto& name : optionNames)
  {
    if (!dynamicOptions.isSame(this->Internals->LibOptions, name))
    {
      // XXX Currently an assert is enough but it should be a proper try/catch once
      // we add a mechanism to unset an option
      assert(dynamicOptions.hasValue(name));
      dynamicOptionsDict[name] = dynamicOptions.getAsString(name);
    }
  }

  // Add the dynamicOptionsDict into the entries, which grows over time if option keep changing and
  // files keep being loaded
  this->Internals->DynamicOptionsEntries.emplace_back(
    dynamicOptionsDict, fs::path(), "dynamic options");

  // Recover file information
  f3d::scene& scene = this->Internals->Engine->getScene();
  bool unsupported = false;

  std::vector<fs::path> localPaths;
  try
  {
    // In the main thread, we only need to guard writing
    const std::lock_guard<std::mutex> lock(this->Internals->LoadedFilesMutex);

    if (clear)
    {
      scene.clear();
      this->Internals->LoadedFiles.clear();
    }

    if (paths.empty())
    {
      f3d::log::debug("No files to load provided");
    }
    else
    {
      // Update app and libf3d options based on config entries, selecting block using the input file
      // config < cli < dynamic
      // Options must be updated before checking the supported files in order to load plugins
      std::vector<fs::path> configPaths = this->Internals->LoadedFiles;
      std::copy(paths.begin(), paths.end(), std::back_inserter(configPaths));
      this->Internals->UpdateOptions(
        { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries,
          this->Internals->DynamicOptionsEntries },
        configPaths);

      this->Internals->Engine->setOptions(this->Internals->LibOptions);
      this->Internals->ApplyPositionAndResolution();

      f3d::log::debug("Checking files:");
      for (const fs::path& tmpPath : paths)
      {
        if (std::find(this->Internals->LoadedFiles.begin(), this->Internals->LoadedFiles.end(),
              tmpPath) == this->Internals->LoadedFiles.end())
        {
          if (scene.supports(tmpPath))
          {
            // Check the size of the file before loading it
            static constexpr int BYTES_IN_MIB = 1048576;
            if (this->Internals->AppOptions.MaxSize >= 0.0 &&
              fs::file_size(tmpPath) >
                static_cast<std::uintmax_t>(this->Internals->AppOptions.MaxSize * BYTES_IN_MIB))
            {
              f3d::log::info(tmpPath.string(), " skipped, file is bigger than max size");
            }
            else
            {
              localPaths.emplace_back(tmpPath);
            }
          }
          else
          {
            f3d::log::warn(tmpPath.string(), " is not a file of a supported file format");
            unsupported = true;
          }
        }
      }

      if (!localPaths.empty())
      {
        // Add files to the scene
        scene.add(localPaths);

        // Update loaded files
        std::copy(
          localPaths.begin(), localPaths.end(), std::back_inserter(this->Internals->LoadedFiles));
      }
    }
  }
  catch (const f3d::scene::load_failure_exception& ex)
  {
    f3d::log::error("Some of these files could not be loaded: ", ex.what());
    for (const fs::path& tmpPath : localPaths)
    {
      f3d::log::error("  ", tmpPath.string());
    }
    unsupported = true;
  }

  std::string filenameInfo;
  if (this->Internals->LoadedFiles.size() > 0)
  {
    // Loaded files, create a filename info like this:
    // "(1/5) cow.vtp + N [+UNSUPPORTED]"
    filenameInfo = groupIdx + " " + this->Internals->LoadedFiles.at(0).filename().string();
    if (this->Internals->LoadedFiles.size() > 1)
    {
      filenameInfo += " +" + std::to_string(this->Internals->LoadedFiles.size() - 1);
    }
    if (unsupported)
    {
      filenameInfo += " [+UNSUPPORTED]";
    }

    // Update dmon watch logic
    if (this->Internals->AppOptions.Watch)
    {
      // Always unwatch and watch current folder, even on reload
      for (const auto& dmonId : this->Internals->FolderWatchIds)
      {
        if (dmonId.id > 0)
        {
          dmon_unwatch(dmonId);
        }
      }
      this->Internals->FolderWatchIds.clear();

      for (const auto& parentPath : F3DInternals::ParentPaths(this->Internals->LoadedFiles))
      {
        this->Internals->FolderWatchIds.emplace_back(
          dmon_watch(parentPath.string().c_str(), &F3DInternals::dmonFolderChanged, 0, this));
      }
    }
  }
  else
  {
    // No files loaded, create a simple filename info like this:
    // (1/5) cow.vtt [UNSUPPORTED]
    // (1/1) cow.vtt [+UNSUPPORTED]
    if (unsupported)
    {
      filenameInfo = groupIdx + " " + paths.at(0).filename().string() + " [";
      if (paths.size() > 1)
      {
        filenameInfo += "+";
      }
      filenameInfo += "UNSUPPORTED]";
    }
  }

  if (!this->Internals->AppOptions.NoRender)
  {
    if (!filenameInfo.empty())
    {
      this->Internals->Engine->getWindow().setWindowName(filenameInfo + " - " + F3D::AppName);
    }

    if (dynamicOptions.scene.camera.index.has_value())
    {
      // Camera is setup by importer, save its configuration as default
      this->Internals->Engine->getWindow().getCamera().setCurrentAsDefault();
    }
    else
    {
      // Setup the camera according to options
      this->Internals->SetupCamera(this->Internals->AppOptions.CamConf);
    }
  }

  // XXX: We can force dropzone and filename_info because they cannot be set
  // manually by the user for now
  f3d::options& options = this->Internals->Engine->getOptions();
  options.ui.dropzone = this->Internals->LoadedFiles.empty();
  options.ui.filename_info = filenameInfo;
}

//----------------------------------------------------------------------------
void F3DStarter::RequestRender()
{
  // Render will be called by the next event loop
  this->Internals->RenderRequested = true;
}

//----------------------------------------------------------------------------
void F3DStarter::Render()
{
  f3d::log::debug("========== Rendering ==========");
  this->Internals->Engine->getWindow().render();
  f3d::log::debug("Render done");
}

//----------------------------------------------------------------------------
void F3DStarter::SaveScreenshot(const std::string& filenameTemplate, bool minimal)
{

  const auto getScreenshotDir = []()
  {
    for (const char* const& candidate : { "XDG_PICTURES_DIR", "HOME", "USERPROFILE" })
    {
      char* val = std::getenv(candidate);
      if (val != nullptr)
      {
        fs::path path(val);
        if (fs::is_directory(path))
        {
          return path;
        }
      }
    }

    return fs::current_path();
  };

  fs::path pathTemplate = fs::path(filenameTemplate).make_preferred();
  fs::path fullPathTemplate =
    pathTemplate.is_absolute() ? pathTemplate : getScreenshotDir() / pathTemplate;
  fs::path path = this->Internals->applyFilenameTemplate(fullPathTemplate.string());

  fs::create_directories(fs::path(path).parent_path());
  f3d::log::info("saving screenshot to " + path.string());

  f3d::options& options = this->Internals->Engine->getOptions();
  f3d::options optionsCopy = this->Internals->Engine->getOptions();

  bool noBackground = this->Internals->AppOptions.NoBackground;
  if (minimal)
  {
    options.ui.scalar_bar = false;
    options.ui.cheatsheet = false;
    options.ui.filename = false;
    options.ui.fps = false;
    options.ui.metadata = false;
    options.ui.animation_progress = false;
    options.interactor.axis = false;
    options.render.grid.enable = false;
    noBackground = true;
  }

  f3d::image img = this->Internals->Engine->getWindow().renderToImage(noBackground);
  this->Internals->addOutputImageMetadata(img);
  img.save(path.string(), f3d::image::SaveFormat::PNG);

  options.render.light.intensity *= 5;
  this->Render();

  this->Internals->Engine->setOptions(optionsCopy);
  this->Render();
}

//----------------------------------------------------------------------------
int F3DStarter::AddFile(const fs::path& path, bool quiet)
{
  // Check file exists
  auto tmpPath = fs::absolute(path);
  if (!fs::exists(tmpPath))
  {
    if (!quiet)
    {
      f3d::log::error("File ", tmpPath.string(), " does not exist");
    }
    return -1;
  }
  // If file is a folder, add files recursively
  else if (fs::is_directory(tmpPath))
  {
    std::set<fs::path> sortedPaths;
    for (const auto& entry : fs::directory_iterator(tmpPath))
    {
      sortedPaths.insert(entry.path());
    }
    for (const auto& entryPath : sortedPaths)
    {
      // Recursively add all files
      this->AddFile(entryPath, quiet);
    }
    return static_cast<int>(this->Internals->FilesGroups.size()) - 1;
  }
  else
  {
    // Check if file has already been added
    bool found = false;
    std::vector<std::vector<fs::path>>::iterator it;
    for (it = this->Internals->FilesGroups.begin(); it != this->Internals->FilesGroups.end(); it++)
    {
      auto localIt = std::find(it->begin(), it->end(), tmpPath);
      found |= localIt != it->end();
      if (found)
      {
        break;
      }
    }

    if (!found)
    {
      // Add to the right file group
      // XXX more multi-file mode may be added in the future
      if (this->Internals->AppOptions.MultiFileMode == "all")
      {
        if (this->Internals->FilesGroups.empty())
        {
          this->Internals->FilesGroups.resize(1);
        }
        assert(this->Internals->FilesGroups.size() == 1);
        this->Internals->FilesGroups[0].emplace_back(tmpPath);
      }
      else
      {
        if (this->Internals->AppOptions.MultiFileMode != "single")
        {
          f3d::log::warn("Unrecognized multi-file-mode: ",
            this->Internals->AppOptions.MultiFileMode, ". Assuming \"single\" mode.");
        }
        this->Internals->FilesGroups.emplace_back(std::vector<fs::path>{ tmpPath });
      }
      return static_cast<int>(this->Internals->FilesGroups.size()) - 1;
    }
    else
    {
      // If already added, just return the index of the group containing the file
      if (!quiet)
      {
        f3d::log::warn("File ", tmpPath.string(), " has already been added");
      }
      return static_cast<int>(std::distance(this->Internals->FilesGroups.begin(), it));
    }
  }
}

//----------------------------------------------------------------------------
bool F3DStarter::LoadRelativeFileGroup(int index, bool restoreCamera, bool forceClear)
{

  if (restoreCamera)
  {
    f3d::camera& cam = this->Internals->Engine->getWindow().getCamera();
    const auto camState = cam.getState();
    this->LoadFileGroup(index, true, forceClear);
    cam.setState(camState);
  }
  else
  {
    this->LoadFileGroup(index, true, forceClear);
  }

  this->RequestRender();

  return true;
}

//----------------------------------------------------------------------------
void F3DStarter::EventLoop()
{
  if (this->Internals->ReloadFileRequested)
  {
    this->LoadRelativeFileGroup(0, true, true);
    this->Internals->ReloadFileRequested = false;
  }
  if (this->Internals->RenderRequested)
  {
    this->Render();
    this->Internals->RenderRequested = false;
  }
}
