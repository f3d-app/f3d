#include "F3DStarter.h"

#include "F3DColorMapTools.h"
#include "F3DConfig.h"
#include "F3DConfigFileTools.h"
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
    bool NoBackground;
    bool NoRender;
    bool cameraIndexPassed;
    double MaxSize;
    bool Watch;
    std::vector<std::string> Plugins;
    std::string ScreenshotFilename;
    std::string VerboseLevel;
    bool GeometryOnly;
    bool GroupGeometries;
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
    const std::lock_guard<std::mutex> lock(self->Internals->FilesListMutex);
    fs::path filePath = self->Internals->FilesList[self->Internals->CurrentFileIndex];
    if (filePath.filename().string() == std::string(filename))
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
        return FilesList[CurrentFileIndex].stem().string();
      }
      else if (var == "model.ext")
      {
        return FilesList[CurrentFileIndex].filename().string();
      }
      else if (var == "model_ext")
      {
        return FilesList[CurrentFileIndex].extension().string().substr(1);
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

  void UpdateOptions(
    const std::vector<F3DOptionsTools::OptionsEntries>& entriesVector, const std::string& inputFile)
  {
    f3d::log::debug("Updating Options:");
    // Initialize libf3dOptions
    f3d::options libOptions;
    libOptions.ui.dropzone_info = "Drop a file or HDRI to load it\nPress H to show cheatsheet";

    // Copy appOptions
    F3DOptionsTools::OptionsDict appOptions = F3DOptionsTools::DefaultAppOptions;

    // Logging specific map
    bool logOptions = this->AppOptions.VerboseLevel == "debug";
    std::map<std::string, std::tuple<std::string, fs::path, std::string, std::string>> loggingMap;

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
            }
            catch (const f3d::options::parsing_exception& ex)
            {
              std::string origin =
                source.empty() ? pattern : source.string() + ":`" + pattern + "`";
              f3d::log::warn("Could not set '", key, "' to '", value, "' from ", origin,
                " because: ", ex.what());
              continue;
            }
            catch (const f3d::options::inexistent_exception&)
            {
              std::string origin =
                source.empty() ? pattern : source.string() + ":`" + pattern + "`";
              auto [closestName, dist] = F3DOptionsTools::GetClosestOption(libf3dOptionName, true);
              f3d::log::warn("'", key, "' option from ", origin,
                " does not exists , did you mean '", closestName, "'?");
              continue;
            }
            if (logOptions)
            {
              loggingMap.emplace(libf3dOptionName, std::tuple(key, source, pattern, value));
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
    this->AppOptions.NoBackground = f3d::options::parse<bool>(appOptions.at("no-background"));
    this->AppOptions.NoRender = f3d::options::parse<bool>(appOptions.at("no-render"));
    this->AppOptions.MaxSize = f3d::options::parse<double>(appOptions.at("max-size"));
    this->AppOptions.Watch = f3d::options::parse<bool>(appOptions.at("watch"));
    this->AppOptions.Plugins = { f3d::options::parse<std::vector<std::string>>(
      appOptions.at("load-plugins")) };
    this->AppOptions.ScreenshotFilename =
      f3d::options::parse<std::string>(appOptions.at("screenshot-filename"));
    this->AppOptions.VerboseLevel = f3d::options::parse<std::string>(appOptions.at("verbose"));
    this->AppOptions.GeometryOnly = f3d::options::parse<bool>(appOptions.at("geometry-only"));
    this->AppOptions.GroupGeometries = f3d::options::parse<bool>(appOptions.at("group-geometries"));
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
      else if (this->AppOptions.Resolution.size() != 0)
      {
        f3d::log::warn("Provided resolution could not be applied");
      }

      if (this->AppOptions.Position.size() == 2)
      {
        window.setPosition(this->AppOptions.Position[0], this->AppOptions.Position[1]);
      }
      else
      {
        if (this->AppOptions.Position.size() != 0)
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

  F3DAppOptions AppOptions;
  f3d::options LibOptions;
  F3DOptionsTools::OptionsEntries ConfigOptionsEntries;
  F3DOptionsTools::OptionsEntries CLIOptionsEntries;
  F3DOptionsTools::OptionsEntries DynamicOptionsEntries;
  std::unique_ptr<f3d::engine> Engine;
  std::vector<fs::path> FilesList;
  dmon_watch_id FolderWatchId;
  bool LoadedFile = false;

  // dmon used atomic and mutex
  std::atomic<int> CurrentFileIndex = -1;
  std::mutex FilesListMutex;

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
  this->Internals->AppOptions.cameraIndexPassed = false;
  if (cliOptionsDict.find("camera-index") != cliOptionsDict.end())
  {
    this->Internals->AppOptions.cameraIndexPassed = true;
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
    { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries }, "");

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  f3d::log::debug("========== Configuring engine ==========");

  const std::string& reference = this->Internals->AppOptions.Reference;
  const std::string& output = this->Internals->AppOptions.Output;

  if (this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine = std::make_unique<f3d::engine>(f3d::window::Type::NONE);
  }
  else
  {
    bool offscreen = !reference.empty() || !output.empty();
    this->Internals->Engine = std::make_unique<f3d::engine>(
      offscreen ? f3d::window::Type::NATIVE_OFFSCREEN : f3d::window::Type::NATIVE);

    f3d::window& window = this->Internals->Engine->getWindow();
    window.setWindowName(F3D::AppTitle).setIcon(F3DIcon, sizeof(F3DIcon));
    this->Internals->ApplyPositionAndResolution();

    f3d::interactor& interactor = this->Internals->Engine->getInteractor();
    interactor.setKeyPressCallBack(
      [this](int, const std::string& keySym) -> bool
      {
        if (keySym == "Left")
        {
          return this->LoadRelativeFile(-1);
        }
        if (keySym == "Right")
        {
          return this->LoadRelativeFile(+1);
        }
        if (keySym == "Up")
        {
          return this->LoadRelativeFile(0, true);
        }
        if (keySym == "Down")
        {
          if (this->Internals->LoadedFile)
          {
            this->AddFile(
              this->Internals->FilesList[static_cast<size_t>(this->Internals->CurrentFileIndex)]
                .parent_path(),
              true);
            return this->LoadRelativeFile(0);
          }
          return true;
        }

        if (keySym == "F12")
        {
          this->SaveScreenshot(this->Internals->AppOptions.ScreenshotFilename);
          return true;
        }
        if (keySym == "F11")
        {
          this->SaveScreenshot(this->Internals->AppOptions.ScreenshotFilename, true);
          return true;
        }

        return false;
      });

    interactor.setDropFilesCallBack(
      [this](const std::vector<std::string>& filesVec) -> bool
      {
        int index = -1;
        for (const std::string& file : filesVec)
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
          this->LoadFile(index);
        }
        this->RequestRender();
        return true;
      });
  }

  this->Internals->Engine->setOptions(this->Internals->LibOptions);
  f3d::log::debug("Engine configured");

  // Add all input files
  for (auto& file : inputFiles)
  {
    this->AddFile(fs::path(file));
  }

  // Load a file
  this->LoadFile();

  if (!this->Internals->AppOptions.NoRender)
  {
    f3d::window& window = this->Internals->Engine->getWindow();
    f3d::interactor& interactor = this->Internals->Engine->getInteractor();

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
      if (!this->Internals->LoadedFile && !noDataForceRender)
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

      if (this->Internals->FilesList.size() > 1 && !this->Internals->AppOptions.GroupGeometries)
      {
        f3d::log::warn("Image comparison was performed using a single 3D file, other provided "
                       "3D files were ignored.");
      }
    }
    // Render to file if needed
    else if (!output.empty())
    {
      if (!this->Internals->LoadedFile && !noDataForceRender)
      {
        f3d::log::error("No file loaded, no rendering performed");
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

      if (this->Internals->FilesList.size() > 1 && !this->Internals->AppOptions.GroupGeometries)
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
      // Create the event loop repeating timer
      interactor.createTimerCallBack(30, [this]() { this->EventLoop(); });
      this->RequestRender();
      interactor.start();
#endif
    }
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void F3DStarter::LoadFile(int index, bool relativeIndex)
{
  // Make sure the animation is stopped before trying to load any file
  if (!this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine->getInteractor().stopAnimation();
  }

  f3d::log::debug("========== Loading 3D file ==========");

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
  f3d::loader& loader = this->Internals->Engine->getLoader();
  fs::path filePath;
  std::string filenameInfo;
  size_t size = this->Internals->FilesList.size();
  if (size != 0)
  {
    if (relativeIndex)
    {
      assert(this->Internals->CurrentFileIndex >= 0);
      this->Internals->CurrentFileIndex += index;
    }
    else
    {
      this->Internals->CurrentFileIndex = index;
    }
    // Create a nice looking filename info eg: "cow.vtp (1/5)"
    // XXX Do not work if CurrentFileIndex + size < 0
    size_t fileIndex = (this->Internals->CurrentFileIndex + size) % size;
    filePath = this->Internals->FilesList[fileIndex];
    filenameInfo = "(" + std::to_string(fileIndex + 1) + "/" +
      std::to_string(this->Internals->FilesList.size()) + ") " + filePath.filename().string();

    this->Internals->CurrentFileIndex = static_cast<int>(fileIndex);
  }
  else
  {
    f3d::log::debug("No file to load provided.");
    this->Internals->CurrentFileIndex = -1;
  }

  if (this->Internals->CurrentFileIndex >= 0)
  {
    if (this->Internals->AppOptions.GroupGeometries)
    {
      // Group geometries mode, consider the first file configuration file only
      this->Internals->CurrentFileIndex = 0;
      filePath = this->Internals->FilesList[static_cast<size_t>(this->Internals->CurrentFileIndex)];
    }

    // Update app and libf3d options based on config entries, selecting block using the input file
    // config < cli < dynamic
    this->Internals->UpdateOptions(
      { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries,
        this->Internals->DynamicOptionsEntries },
      filePath.string());
    this->Internals->Engine->setOptions(this->Internals->LibOptions);
    this->Internals->ApplyPositionAndResolution();

    // Load any new plugins the updated app options
    F3DPluginsTools::LoadPlugins(this->Internals->AppOptions.Plugins);

    // Position the loaded file flag before we start loading
    this->Internals->LoadedFile = false;

    // Check the size of the file before loading it
    // XXX: Not considered in the context of GroupGeometries
    static constexpr int BYTES_IN_MIB = 1048576;
    if (this->Internals->AppOptions.MaxSize >= 0.0 &&
      fs::file_size(filePath) >
        static_cast<std::uintmax_t>(this->Internals->AppOptions.MaxSize * BYTES_IN_MIB))
    {
      f3d::log::info("No file loaded, file is bigger than max size");
    }
    else
    {
      try
      {
        if (loader.hasSceneReader(filePath.string()) && !this->Internals->AppOptions.GeometryOnly &&
          !this->Internals->AppOptions.GroupGeometries)
        {
          loader.loadScene(filePath.string());
          this->Internals->LoadedFile = true;
        }
        else if (loader.hasGeometryReader(filePath.string()))
        {
          // In GroupGeometries, just load all the files from the list
          if (this->Internals->AppOptions.GroupGeometries)
          {
            int nGeom = 0;
            for (size_t i = 0; i < size; i++)
            {
              auto geomPath = this->Internals->FilesList[i];
              if (loader.hasGeometryReader(geomPath.string()))
              {
                // Reset for the first file, then add geometries without resetting
                // XXX this means the scene is reset and loaded from scratch every time a file is
                // dropped This could be improved
                loader.loadGeometry(this->Internals->FilesList[i].string(), i == 0 ? true : false);
                nGeom++;
              }
              else
              {
                f3d::log::warn(geomPath, " is not a geometry of a supported file format\n");
              }
            }
            if (nGeom > 1)
            {
              filenameInfo = std::to_string(nGeom) + " geometries loaded";
            }
          }
          else
          {
            // Standard loadGeometry code
            loader.loadGeometry(filePath.string(), true);
          }
          this->Internals->LoadedFile = true;
        }
        else
        {
          f3d::log::debug("No reader found for \"" + filePath.string() + "\"");
          f3d::log::warn(filePath.string(), " is not a file of a supported file format\n");
          filenameInfo += " [UNSUPPORTED]";
        }
      }
      catch (const f3d::loader::load_failure_exception& ex)
      {
        // XXX Not reachable until vtkImporter is improved to support returning a failure
        f3d::log::error("Could not load file: ", ex.what());
      }

      if (!dynamicOptions.scene.camera.index.has_value() && !this->Internals->AppOptions.NoRender)
      {
        // Setup the camera according to options
        this->Internals->SetupCamera(this->Internals->AppOptions.CamConf);

        this->Internals->Engine->getWindow().setWindowName(
          filePath.filename().string() + " - " + F3D::AppName);
      }
    }
  }

  if (this->Internals->LoadedFile)
  {
    if (this->Internals->AppOptions.Watch)
    {
      // Always unwatch and watch current folder, even on reload
      if (this->Internals->FolderWatchId.id > 0)
      {
        dmon_unwatch(this->Internals->FolderWatchId);
      }
      this->Internals->FolderWatchId = dmon_watch(
        filePath.parent_path().string().c_str(), &F3DInternals::dmonFolderChanged, 0, this);
    }
  }
  else
  {
    // No file loaded, remove any previously loaded file
    loader.loadGeometry("", true);
  }

  // XXX: We can force dropzone and filename_info because they cannot be set
  // manually by the user for now
  f3d::options& options = this->Internals->Engine->getOptions();
  options.ui.dropzone = !this->Internals->LoadedFile;
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
  auto tmpPath = fs::absolute(path);
  if (!fs::exists(tmpPath))
  {
    if (!quiet)
    {
      f3d::log::error("File ", tmpPath.string(), " does not exist");
    }
    return -1;
  }
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
    return static_cast<int>(this->Internals->FilesList.size()) - 1;
  }
  else
  {
    auto it =
      std::find(this->Internals->FilesList.begin(), this->Internals->FilesList.end(), tmpPath);

    if (it == this->Internals->FilesList.end())
    {
      // In the main thread, we only need to guard writing
      const std::lock_guard<std::mutex> lock(this->Internals->FilesListMutex);
      this->Internals->FilesList.push_back(tmpPath);
      return static_cast<int>(this->Internals->FilesList.size()) - 1;
    }
    else
    {
      if (!quiet)
      {
        f3d::log::warn("File ", tmpPath.string(), " has already been added");
      }
      return static_cast<int>(std::distance(this->Internals->FilesList.begin(), it));
    }
  }
}

//----------------------------------------------------------------------------
bool F3DStarter::LoadRelativeFile(int index, bool restoreCamera)
{

  if (restoreCamera)
  {
    f3d::camera& cam = this->Internals->Engine->getWindow().getCamera();
    const auto camState = cam.getState();
    this->LoadFile(index, true);
    cam.setState(camState);
  }
  else
  {
    this->LoadFile(index, true);
  }

  this->RequestRender();

  return true;
}

//----------------------------------------------------------------------------
void F3DStarter::EventLoop()
{
  if (this->Internals->ReloadFileRequested)
  {
    this->LoadRelativeFile(0, true);
    this->Internals->ReloadFileRequested = false;
  }
  if (this->Internals->RenderRequested)
  {
    this->Render();
    this->Internals->RenderRequested = false;
  }
}
