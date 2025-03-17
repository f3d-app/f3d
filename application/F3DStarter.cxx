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

#if F3D_MODULE_DMON
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
#endif

#if F3D_MODULE_TINYFILEDIALOGS
#include "tinyfiledialogs.h"
#endif

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "options.h"
#include "utils.h"
#include "window.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <regex>
#include <set>

namespace fs = std::filesystem;

// This pointer is used to retrieve the interactor in case an OS signal is handled
f3d::interactor* GlobalInteractor = nullptr;

class F3DStarter::F3DInternals
{
public:
  F3DInternals() = default;

  using log_entry_t = std::tuple<std::string, std::string, std::string, std::string>;

  // XXX: The values in the following two structs
  // are left uninitialized as the will all be initialized from
  // F3DOptionsTools::DefaultAppOptions
  struct CameraConfiguration
  {
    std::vector<double> CameraPosition;
    std::vector<double> CameraFocalPoint;
    std::optional<f3d::direction_t> CameraViewUp;
    double CameraViewAngle;
    std::optional<f3d::direction_t> CameraDirection;
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
    std::optional<double> MaxSize;
    std::optional<double> AnimationTime;
    bool Watch;
    double FrameRate;
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
    std::string CommandScriptFile;
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
    if (camConf.CameraViewUp.has_value())
    {
      cam.setViewUp(camConf.CameraViewUp.value());
    }
    if (camConf.CameraViewAngle > 0)
    {
      cam.setViewAngle(camConf.CameraViewAngle);
    }

    bool reset = false;
    double zoomFactor = 0.9;
    if (camConf.CameraPosition.size() != 3 && camConf.CameraDirection.has_value())
    {
      f3d::vector3_t dir = camConf.CameraDirection.value();
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
    if (camConf.CameraPosition.size() != 3)
    {
      if (camConf.CameraZoomFactor > 0)
      {
        zoomFactor = camConf.CameraZoomFactor;
      }
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

#if F3D_MODULE_DMON
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
#endif

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
      cameraMetadata << "  \"position\": " << vec3toJson(state.position) << ",\n";
      cameraMetadata << "  \"focalPoint\": " << vec3toJson(state.focalPoint) << ",\n";
      cameraMetadata << "  \"viewUp\": " << vec3toJson(state.viewUp) << ",\n";
      cameraMetadata << "  \"viewAngle\": " << state.viewAngle << "\n";
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
  fs::path applyFilenameTemplate(const fs::path& templatePath)
  {
    constexpr size_t maxNumberingAttempts = 1000000;
    const std::regex numberingRe("(n:?(.*))");
    const std::regex dateRe("date:?(.*)");

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
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), fmt.c_str());
        std::string formatted = ss.str();
        if (formatted == fmt)
        {
          f3d::log::warn("invalid date format for \"", var, "\"");
        }
        return formatted;
      }
      throw f3d::utils::string_template::lookup_error(var);
    };

    f3d::utils::string_template stringTemplate(templatePath.string());
    stringTemplate.substitute(variableLookup);

    const auto hasNumbering = [&]()
    {
      for (const auto& variable : stringTemplate.variables())
      {
        if (std::regex_search(variable, numberingRe))
        {
          return true;
        }
      }
      return false;
    };

    /* return if there's no numbering to be done */
    if (!hasNumbering())
    {
      return { stringTemplate.str() };
    }

    const auto numberingLookup = [&](const size_t number)
    {
      return [&numberingRe, number](const std::string& var)
      {
        if (std::regex_match(var, numberingRe))
        {
          std::stringstream formattedNumber;
          const std::string fmt = std::regex_replace(var, numberingRe, "$2");
          try
          {
            formattedNumber << std::setfill('0') << std::setw(std::stoi(fmt)) << number;
          }
          catch (std::invalid_argument&)
          {
            if (!fmt.empty() && number == 1) /* avoid spamming the log */
            {
              f3d::log::warn("ignoring invalid number format for \"", var, "\"");
            }
            formattedNumber << std::setw(0) << number;
          }
          return std::regex_replace(var, numberingRe, formattedNumber.str());
        }
        throw f3d::utils::string_template::lookup_error(var);
      };
    };

    /* try substituting incrementing number until file doesn't exist already */
    for (size_t i = 1; i <= maxNumberingAttempts; ++i)
    {
      const std::string candidate =
        f3d::utils::string_template(stringTemplate).substitute(numberingLookup(i)).str();
      if (!fs::exists(candidate))
      {
        return { candidate };
      }
    }
    throw std::runtime_error("could not find available unique filename after " +
      std::to_string(maxNumberingAttempts) + " attempts");
  }

  static void PrintLoggingMap(const std::map<std::string, log_entry_t>& loggingMap, char sep)
  {
    for (const auto& [key, tuple] : loggingMap)
    {
      const auto& [bindStr, source, pattern, commands] = tuple;
      std::string origin = source.empty() ? pattern : std::string(source) + ":`" + pattern + "`";
      f3d::log::debug(" '", bindStr, "' ", sep, " '", commands, "' from ", origin);
    }
    f3d::log::debug("");
  }

  void UpdateOptions(const std::vector<F3DOptionsTools::OptionsEntries>& entriesVector,
    const std::vector<fs::path>& paths, bool quiet)
  {
    assert(!paths.empty());

    if (!quiet)
    {
      f3d::log::debug("Updating Options:");
    }

    // Initialize libf3dOptions
    f3d::options libOptions;
    libOptions.ui.dropzone_info = "Drop a file or HDRI to load it\nPress H to show cheatsheet";

    // Copy appOptions
    F3DOptionsTools::OptionsDict appOptions = F3DOptionsTools::DefaultAppOptions;

    // Logging specific map
    bool logOptions = this->AppOptions.VerboseLevel == "debug" && !quiet;
    std::map<std::string, log_entry_t> loggingMap;

    // For each input file, order matters
    for (const auto& tmpPath : paths)
    {
      std::string inputFile = tmpPath.string();
      // For each config entries, ordered by priority
      for (const auto& entries : entriesVector)
      {
        // For each entry (eg: different config files)
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
                  loggingMap[key] = std::tuple(key, source, pattern, value);
                }
                continue;
              }

              // Convert key into a libf3d option name if possible
              std::string libf3dOptionName = key;
              std::string keyForLog = key;
              auto libf3dIter = F3DOptionsTools::LibOptionsNames.find(libf3dOptionName);
              if (libf3dIter != F3DOptionsTools::LibOptionsNames.end())
              {
                libf3dOptionName = std::string(libf3dIter->second);
              }

              std::string libf3dOptionValue = value;
              bool reset = false;

              // Handle options reset
              // XXX: Use starts_with once C++20 is supported
              if (libf3dOptionName.rfind("reset-", 0) == 0)
              {
                reset = true;
                libf3dOptionName = libf3dOptionName.substr(6);
                keyForLog = libf3dOptionName;
                libf3dOptionValue = "reset";
              }

              // Handle reader options
              std::vector<std::string> readerOptionNames = f3d::engine::getAllReaderOptionNames();
              if (std::find(readerOptionNames.begin(), readerOptionNames.end(), libf3dOptionName) !=
                readerOptionNames.end())
              {
                f3d::engine::setReaderOption(libf3dOptionName, libf3dOptionValue);
                continue;
              }

              try
              {
                // Assume this is a libf3d option and set/reset the value
                if (reset)
                {
                  libOptions.reset(libf3dOptionName);
                }
                else
                {
                  libOptions.setAsString(libf3dOptionName, libf3dOptionValue);
                }

                // Log the option if needed
                if (logOptions)
                {
                  loggingMap[libf3dOptionName] =
                    std::tuple(keyForLog, source, pattern, libf3dOptionValue);
                }
              }
              catch (const f3d::options::parsing_exception& ex)
              {
                if (!quiet)
                {
                  std::string origin =
                    source.empty() ? pattern : std::string(source) + ":`" + pattern + "`";
                  f3d::log::warn("Could not set '", keyForLog, "' to '", libf3dOptionValue,
                    "' from ", origin, " because: ", ex.what());
                }
              }
              catch (const f3d::options::inexistent_exception&)
              {
                if (!quiet)
                {
                  std::string origin =
                    source.empty() ? pattern : std::string(source) + ":`" + pattern + "`";
                  auto [closestName, dist] =
                    F3DOptionsTools::GetClosestOption(libf3dOptionName, true);
                  f3d::log::warn("'", keyForLog, "' option from ", origin,
                    " does not exists , did you mean '", closestName, "'?");
                }
              }
            }
          }
        }
      }
    }

    F3DInternals::PrintLoggingMap(loggingMap, '=');

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

  void UpdateTypedAppOptions(const F3DOptionsTools::OptionsDict& appOptions)
  {
    // Update typed app options from app options
    this->AppOptions.Output = f3d::options::parse<std::string>(appOptions.at("output"));
    this->AppOptions.BindingsList = f3d::options::parse<bool>(appOptions.at("list-bindings"));
    this->AppOptions.NoBackground = f3d::options::parse<bool>(appOptions.at("no-background"));
    this->AppOptions.NoRender = f3d::options::parse<bool>(appOptions.at("no-render"));
    this->AppOptions.RenderingBackend =
      f3d::options::parse<std::string>(appOptions.at("rendering-backend"));

    std::string maxSize = appOptions.at("max-size");
    if (!maxSize.empty())
    {
      this->AppOptions.MaxSize = f3d::options::parse<double>(maxSize);
    }

    std::string animationTime = appOptions.at("animation-time");
    if (!animationTime.empty())
    {
      this->AppOptions.AnimationTime = f3d::options::parse<double>(animationTime);
    }

    this->AppOptions.FrameRate = f3d::options::parse<double>(appOptions.at("frame-rate"));
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

    std::optional<f3d::direction_t> camDir;
    std::string camDirStr = appOptions.at("camera-direction");
    if (!camDirStr.empty())
    {
      camDir = f3d::options::parse<f3d::direction_t>(camDirStr);
    }

    std::optional<f3d::direction_t> camUp;
    std::string camUpStr = appOptions.at("camera-view-up");
    if (!camUpStr.empty())
    {
      camUp = f3d::options::parse<f3d::direction_t>(camUpStr);
    }

    this->AppOptions.CamConf = { f3d::options::parse<std::vector<double>>(
                                   appOptions.at("camera-position")),
      f3d::options::parse<std::vector<double>>(appOptions.at("camera-focal-point")), camUp,
      f3d::options::parse<double>(appOptions.at("camera-view-angle")), camDir,
      f3d::options::parse<double>(appOptions.at("camera-zoom-factor")),
      f3d::options::parse<double>(appOptions.at("camera-azimuth-angle")),
      f3d::options::parse<double>(appOptions.at("camera-elevation-angle")) };

    this->AppOptions.Reference = f3d::options::parse<std::string>(appOptions.at("reference"));
    this->AppOptions.RefThreshold =
      f3d::options::parse<double>(appOptions.at("reference-threshold"));
    this->AppOptions.InteractionTestRecordFile =
      f3d::options::parse<std::string>(appOptions.at("interaction-test-record"));
    this->AppOptions.InteractionTestPlayFile =
      f3d::options::parse<std::string>(appOptions.at("interaction-test-play"));
    this->AppOptions.CommandScriptFile =
      f3d::options::parse<std::string>(appOptions.at("command-script"));
  }

  void UpdateInterdependentOptions()
  {
    // colormap-file and colormap are interdependent
    const std::string& colorMapFile = this->AppOptions.ColorMapFile;
    if (!colorMapFile.empty())
    {
      fs::path fullPath = F3DColorMapTools::Find(colorMapFile);

      if (!fullPath.empty())
      {
        this->LibOptions.model.scivis.colormap = F3DColorMapTools::Read(fullPath);
      }
      else
      {
        f3d::log::error("Cannot find the colormap ", colorMapFile);
        this->LibOptions.model.scivis.colormap = f3d::colormap_t();
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

  // Update bindings by initializing them and adding the app and config bindings
  void UpdateBindings(const std::vector<fs::path>& paths)
  {
    if (!this->AppOptions.NoRender)
    {
      // std::vector<std::string> -> std::string for printing
      auto vecToString = [](const std::vector<std::string>& vec)
      {
        std::string ret;
        for (const std::string& val : vec)
        {
          ret += "`";
          ret += val;
          ret += "` ";
        }
        return ret;
      };

      // "doc", ""
      auto docString = [](const std::string& doc) { return std::make_pair(doc, ""); };

      auto docStringVec = [&](const std::vector<std::string>& docs)
      { return std::make_pair(vecToString(docs), ""); };

      using mod_t = f3d::interaction_bind_t::ModifierKeys;

      f3d::interactor& interactor = this->Engine->getInteractor();
      interactor.initBindings();

      // clang-format off
      interactor.addBinding({ mod_t::NONE, "Left" }, "load_previous_file_group", "Others", std::bind(docString, "Load previous file group"));
      interactor.addBinding({ mod_t::NONE, "Right" }, "load_next_file_group", "Others", std::bind(docString, "Load next file group"));
      interactor.addBinding({ mod_t::CTRL, "Left" }, "load_previous_file_group true", "Others", std::bind(docString, "Load previous file group, keeping camera"));
      interactor.addBinding({ mod_t::CTRL, "Right" }, "load_next_file_group true", "Others", std::bind(docString, "Load next file group, keeping camera"));
      interactor.addBinding({ mod_t::NONE, "Up" }, "reload_current_file_group", "Others", std::bind(docString, "Reload current file group"));
      interactor.addBinding({ mod_t::NONE, "Down" }, "add_current_directories", "Others", std::bind(docString, "Add files from dir of current file"));
      interactor.addBinding({ mod_t::NONE, "F12" }, "take_screenshot", "Others", std::bind(docString, "Take a screenshot"));
#if F3D_MODULE_TINYFILEDIALOGS
      interactor.addBinding({ mod_t::CTRL, "O" }, "open_file_dialog", "Others", std::bind(docString, "Open File Dialog"));
#endif
      interactor.addBinding({ mod_t::CTRL, "F12" }, "take_minimal_screenshot", "Others", std::bind(docString, "Take a minimal screenshot"));

      // This replace an existing default binding command in the libf3d
      interactor.removeBinding({ mod_t::NONE, "Drop" });
      interactor.addBinding({ mod_t::NONE, "Drop" }, "add_files_or_set_hdri", "Others", std::bind(docString, "Load dropped files, folder or HDRI"));
      interactor.addBinding({ mod_t::CTRL, "Drop" }, "add_files", "Others", std::bind(docString, "Load dropped files or folder"));
      interactor.addBinding({ mod_t::SHIFT, "Drop" }, "set_hdri", "Others", std::bind(docString, "Set HDRI and use it"));

      interactor.removeBinding({mod_t::CTRL, "Q"});
      interactor.addBinding({mod_t::CTRL, "Q"}, "exit", "Others", std::bind(docString, "Quit"));
      // clang-format on

      f3d::log::debug("Adding config defined bindings if any: ");
      bool logBindings = this->AppOptions.VerboseLevel == "debug";
      std::map<std::string, log_entry_t> loggingMap;

      // For each input file, order matters
      for (const auto& tmpPath : paths)
      {
        std::string inputFile = tmpPath.string();
        for (auto const& [bindings, source, pattern] : this->ConfigBindingsEntries)
        {
          std::regex re(pattern, std::regex_constants::icase);
          std::smatch matches;

          // If the source is empty, there is no pattern, all bindings applies
          // Note: An empty inputFile matches with ".*"
          if (source.empty() || std::regex_match(inputFile, matches, re))
          {
            // For each interaction bindings
            for (auto const& [bindStr, commands] : bindings)
            {
              if (logBindings)
              {
                // XXX: Formatting could be improved
                loggingMap.emplace(
                  bindStr, std::tuple(bindStr, source, pattern, vecToString(commands)));
              }

              f3d::interaction_bind_t bind = f3d::interaction_bind_t::parse(bindStr);
              interactor.removeBinding(bind);
              interactor.addBinding(bind, commands, "Config", std::bind(docStringVec, commands));
            }
          }
        }
      }
      F3DInternals::PrintLoggingMap(loggingMap, ':');
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

  static void SigCallback(int)
  {
    if (GlobalInteractor)
    {
      GlobalInteractor->stop();
      GlobalInteractor = nullptr;
    }
  }

  F3DAppOptions AppOptions;
  f3d::options LibOptions;
  F3DOptionsTools::OptionsEntries ConfigOptionsEntries;
  F3DOptionsTools::OptionsEntries CLIOptionsEntries;
  F3DOptionsTools::OptionsEntries DynamicOptionsEntries;
  F3DOptionsTools::OptionsEntries ImperativeConfigOptionsEntries;
  F3DConfigFileTools::BindingsEntries ConfigBindingsEntries;
  std::unique_ptr<f3d::engine> Engine;
  std::vector<std::vector<fs::path>> FilesGroups;
  std::vector<fs::path> LoadedFiles;
  int CurrentFilesGroupIndex = -1;

#if F3D_MODULE_DMON
  // dmon related
  std::mutex LoadedFilesMutex;
  std::vector<dmon_watch_id> FolderWatchIds;
#endif

  // Event loop atomics
  std::atomic<bool> ReloadFileRequested = false;
};

//----------------------------------------------------------------------------
F3DStarter::F3DStarter()
  : Internals(std::make_unique<F3DStarter::F3DInternals>())
{
#if F3D_MODULE_DMON
  // Initialize dmon
  dmon_init();
#endif
}

//----------------------------------------------------------------------------
F3DStarter::~F3DStarter()
{
#if F3D_MODULE_DMON
  // deinit dmon
  dmon_deinit();
#endif
}

//----------------------------------------------------------------------------
int F3DStarter::Start(int argc, char** argv)
{
  // Parse CLI Options into an option dict
  std::vector<std::string> inputFiles;
  F3DOptionsTools::OptionsDict cliOptionsDict =
    F3DOptionsTools::ParseCLIOptions(argc, argv, inputFiles);

  // Store in a option entries for easier processing
  this->Internals->CLIOptionsEntries.emplace_back(cliOptionsDict, "", "CLI options");

  // Check no-config, config CLI, output and verbose options first
  // XXX: the local variable are initialized manually for simplicity
  // but this duplicate the initialization value as it is present in
  // F3DOptionTools::DefaultAppOptions too
  F3DOptionsTools::OptionsDict::const_iterator iter;

  bool noConfig = false;
  iter = cliOptionsDict.find("no-config");
  if (iter != cliOptionsDict.end())
  {
    noConfig = f3d::options::parse<bool>(iter->second);
  }

  std::string config;
  if (!noConfig)
  {
    iter = cliOptionsDict.find("config");
    if (iter != cliOptionsDict.end())
    {
      config = f3d::options::parse<std::string>(iter->second);
    }
  }

  bool renderToStdout = false;
  iter = cliOptionsDict.find("output");
  if (iter != cliOptionsDict.end())
  {
    renderToStdout = f3d::options::parse<std::string>(iter->second) == "-";
  }

  this->Internals->AppOptions.VerboseLevel = "info";
  iter = cliOptionsDict.find("verbose");
  if (iter != cliOptionsDict.end())
  {
    this->Internals->AppOptions.VerboseLevel = f3d::options::parse<std::string>(iter->second);
  }

  // Set verbosity level early from command line
  F3DInternals::SetVerboseLevel(this->Internals->AppOptions.VerboseLevel, renderToStdout);

  f3d::log::debug("========== Initializing Options ==========");

  // Read config files
  if (!noConfig)
  {
    F3DConfigFileTools::ParsedConfigFiles parsedConfigFiles =
      F3DConfigFileTools::ReadConfigFiles(config);
    this->Internals->ConfigOptionsEntries = parsedConfigFiles.Options;
    this->Internals->ImperativeConfigOptionsEntries = parsedConfigFiles.ImperativeOptions;
    this->Internals->ConfigBindingsEntries = parsedConfigFiles.Bindings;
  }

  // Update app and libf3d options based on config entries, with an empty input file
  // config < cli.
  // Force it to be quiet has another options update happens later.
  this->Internals->UpdateOptions(
    { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries,
      this->Internals->ImperativeConfigOptionsEntries },
    { "" }, true);

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  f3d::log::debug("========== Configuring engine ==========");

  double deltaTime = 1.0 / this->Internals->AppOptions.FrameRate;

  if (this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::createNone());
  }
  else
  {
    bool offscreen = !this->Internals->AppOptions.Reference.empty() ||
      !this->Internals->AppOptions.Output.empty() || this->Internals->AppOptions.BindingsList;

    try
    {
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
    }
    catch (const f3d::context::loading_exception& ex)
    {
      f3d::log::error("Could not load graphic library: ", ex.what());
      return EXIT_FAILURE;
    }
    catch (const f3d::context::symbol_exception& ex)
    {
      f3d::log::error("Could not find needed symbol in graphic library: ", ex.what());
      return EXIT_FAILURE;
    }
    catch (const f3d::engine::no_window_exception& ex)
    {
      f3d::log::error("Could not create the window: ", ex.what());
      return EXIT_FAILURE;
    }
    catch (const f3d::engine::cache_exception& ex)
    {
      f3d::log::error("Could not use default cache directory: ", ex.what());
      return EXIT_FAILURE;
    }

    this->ResetWindowName();
    this->Internals->ApplyPositionAndResolution();
    this->AddCommands();
    this->Internals->UpdateBindings({ "" });
  }

  this->Internals->Engine->setOptions(this->Internals->LibOptions);
  f3d::log::debug("Engine configured");

  // Add all input files
  for (auto& file : inputFiles)
  {
    this->AddFile(f3d::utils::collapsePath(file));
  }

  // Load a file
  this->LoadFileGroup();

  if (!this->Internals->AppOptions.NoRender)
  {
    this->Internals->ApplyPositionAndResolution();
    f3d::window& window = this->Internals->Engine->getWindow();
    f3d::interactor& interactor = this->Internals->Engine->getInteractor();

    // Print bindings list and exits if needed
    if (this->Internals->AppOptions.BindingsList)
    {
      f3d::log::info("Bindings:");
      for (const std::string& group : interactor.getBindGroups())
      {
        f3d::log::info(" ", group, ":");
        for (const f3d::interaction_bind_t& bind : interactor.getBindsForGroup(group))
        {
          // XXX: Formatting could be improved here
          auto [doc, val] = interactor.getBindingDocumentation(bind);
          F3DOptionsTools::PrintHelpPair(bind.format(), doc, 12);
        }
        f3d::log::info("");
      }
      throw F3DExNoProcess("bindings list requested");
    }

    // Play recording if any
    fs::path interactionTestPlayFile =
      f3d::utils::collapsePath(this->Internals->AppOptions.InteractionTestPlayFile);
    if (!interactionTestPlayFile.empty())
    {
      // For better testing, render once before the interaction
      window.render();
      if (!interactor.playInteraction(interactionTestPlayFile, deltaTime))
      {
        return EXIT_FAILURE;
      }
    }

    // Start recording if needed
    fs::path interactionTestRecordFile =
      f3d::utils::collapsePath(this->Internals->AppOptions.InteractionTestRecordFile);
    if (!interactionTestRecordFile.empty())
    {
      if (!interactor.recordInteraction(interactionTestRecordFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Process Command Script file
    fs::path commandScriptFile =
      f3d::utils::collapsePath(this->Internals->AppOptions.CommandScriptFile);
    if (!commandScriptFile.empty())
    {
      std::ifstream scriptFile(commandScriptFile);
      if (scriptFile.is_open())
      {
        std::string command;
        while (std::getline(scriptFile, command))
        {
          if (!command.empty())
          {
            // XXX: No need to catch interactor::command_runtime_exception
            // as neither libf3d nor F3D has command that can trigger it
            if (!interactor.triggerCommand(command))
            {
              f3d::log::error("Error in command script, stopping script execution");
              break;
            }
          }
        }
        scriptFile.close();
      }
      else
      {
        f3d::log::error("Unable to open command script file");
        return EXIT_FAILURE;
      }
    }

    char* noDataForceRender = std::getenv("CTEST_F3D_NO_DATA_FORCE_RENDER");

    fs::path reference = f3d::utils::collapsePath(this->Internals->AppOptions.Reference);
    fs::path output = this->Internals->applyFilenameTemplate(
      f3d::utils::collapsePath(this->Internals->AppOptions.Output));

    // Render and compare with file if needed
    if (!reference.empty())
    {
      if (this->Internals->LoadedFiles.empty() && !noDataForceRender)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      try
      {
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
            try
            {
              window.renderToImage(this->Internals->AppOptions.NoBackground).save(output);
            }
            catch (const f3d::image::write_exception& ex)
            {
              f3d::log::error("Could not write output: ", ex.what());
              return EXIT_FAILURE;
            }

            f3d::log::error("Reference image ", reference,
              " does not exist, current rendering has been outputted to ", output, ".\n");
          }
          return EXIT_FAILURE;
        }
      }
      catch (const fs::filesystem_error& ex)
      {
        f3d::log::error("Error reading reference image: ", ex.what());
        return EXIT_FAILURE;
      }

      f3d::image img = window.renderToImage(this->Internals->AppOptions.NoBackground);
      f3d::image ref(reference);
      f3d::image diff;
      double error = img.compare(ref);
      const double& threshold = this->Internals->AppOptions.RefThreshold;
      if (error > threshold)
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

          try
          {
            img.save(output);
          }
          catch (const f3d::image::write_exception& ex)
          {
            f3d::log::error("Could not write output: ", ex.what());
            return EXIT_FAILURE;
          }
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
        try
        {
          img.save(output);
        }
        catch (const f3d::image::write_exception& ex)
        {
          f3d::log::error("Could not write output: ", ex.what());
          return EXIT_FAILURE;
        }

        f3d::log::debug("Output image saved to ", output);
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
        window.render();

        // gracefully exits if SIGTERM or SIGINT is send to F3D
        GlobalInteractor = &interactor;
        std::signal(SIGTERM, F3DInternals::SigCallback);
        std::signal(SIGINT, F3DInternals::SigCallback);

        interactor.start(deltaTime, [this]() { this->EventLoop(); });
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
  f3d::options& dynamicOptions = this->Internals->Engine->getOptions();

  // reset forced options to avoid logging noise
  dynamicOptions.ui.dropzone = false;
  dynamicOptions.ui.filename_info = "";

  // Detect interactively changed options and store them into the dynamic options dict
  // options names are shared between options instance
  F3DOptionsTools::OptionsDict dynamicOptionsDict;
  std::vector<std::string> optionNames = dynamicOptions.getAllNames();
  for (const auto& name : optionNames)
  {
    if (!dynamicOptions.isSame(this->Internals->LibOptions, name))
    {
      if (!dynamicOptions.hasValue(name))
      {
        // If a dynamic option has been changed and does not have value, it means it was reset using
        // the command line reset it using the dedicated syntax
        dynamicOptionsDict["reset-" + name] = "";
      }
      else
      {
        // No need for a try/catch block here, this call cannot trigger
        // an exception with current code path
        dynamicOptionsDict[name] = dynamicOptions.getAsString(name);
      }
    }
  }

  // Add the dynamicOptionsDict into the entries, which grows over time if option keep changing and
  // files keep being loaded
  this->Internals->DynamicOptionsEntries.emplace_back(dynamicOptionsDict, "", "dynamic options");

  // Recover file information
  f3d::scene& scene = this->Internals->Engine->getScene();
  bool unsupported = false;

  std::vector<fs::path> localPaths;
  try
  {
#if F3D_MODULE_DMON
    // In the main thread, we only need to guard writing
    const std::lock_guard<std::mutex> lock(this->Internals->LoadedFilesMutex);
#endif

    if (clear)
    {
      scene.clear();
      this->Internals->LoadedFiles.clear();
    }

    if (paths.empty())
    {
      // Update options even when there is no file
      // as imperative options should override dynamic option even in that case
      this->Internals->UpdateOptions(
        { this->Internals->ConfigOptionsEntries, this->Internals->CLIOptionsEntries,
          this->Internals->DynamicOptionsEntries, this->Internals->ImperativeConfigOptionsEntries },
        { "" }, false);
      this->Internals->Engine->setOptions(this->Internals->LibOptions);
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
          this->Internals->DynamicOptionsEntries, this->Internals->ImperativeConfigOptionsEntries },
        configPaths, false);
      this->Internals->UpdateBindings(configPaths);

      this->Internals->Engine->setOptions(this->Internals->LibOptions);

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
            if (this->Internals->AppOptions.MaxSize.has_value() &&
              fs::file_size(tmpPath) >
                static_cast<std::uintmax_t>(
                  this->Internals->AppOptions.MaxSize.value() * BYTES_IN_MIB))
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

        if (this->Internals->AppOptions.AnimationTime.has_value())
        {
          f3d::log::debug(
            "Loading animation time: ", this->Internals->AppOptions.AnimationTime.value());
          scene.loadAnimationTime(this->Internals->AppOptions.AnimationTime.value());
        }

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

#if F3D_MODULE_DMON
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
#endif
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

  // XXX: Here we potentially override user set libf3d options
  // but there is no way to detect if an option has been set
  // by the user or not.
  f3d::options& options = this->Internals->Engine->getOptions();
  options.ui.dropzone = this->Internals->LoadedFiles.empty();
  options.ui.filename_info = filenameInfo;
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

  fs::path path;
  try
  {
    path = this->Internals->applyFilenameTemplate(
      f3d::utils::collapsePath(filenameTemplate, getScreenshotDir()));

    fs::create_directories(path.parent_path());
    f3d::log::info("saving screenshot to " + path.string());
  }
  catch (const fs::filesystem_error& ex)
  {
    f3d::log::error("Error recovering screenshot path: ", ex.what());
    return;
  }

  f3d::options& options = this->Internals->Engine->getOptions();
  f3d::options optionsCopy = this->Internals->Engine->getOptions();

  bool noBackground = this->Internals->AppOptions.NoBackground;
  if (minimal)
  {
    options.ui.scalar_bar = false;
    options.ui.cheatsheet = false;
    options.ui.console = false;
    options.ui.filename = false;
    options.ui.fps = false;
    options.ui.metadata = false;
    options.ui.animation_progress = false;
    options.ui.axis = false;
    options.render.grid.enable = false;
    noBackground = true;
  }

  f3d::image img = this->Internals->Engine->getWindow().renderToImage(noBackground);
  this->Internals->addOutputImageMetadata(img);
  img.save(path, f3d::image::SaveFormat::PNG);

  options.render.light.intensity *= 5;
  this->Render();

  this->Internals->Engine->setOptions(optionsCopy);
  this->Render();
}

//----------------------------------------------------------------------------
int F3DStarter::AddFile(const fs::path& path, bool quiet)
{
  try
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
      for (it = this->Internals->FilesGroups.begin(); it != this->Internals->FilesGroups.end();
           it++)
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
  catch (const fs::filesystem_error& ex)
  {
    if (!quiet)
    {
      f3d::log::error("Error adding file: ", ex.what());
    }
    return -1;
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

  this->Internals->Engine->getInteractor().requestRender();

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
}

//----------------------------------------------------------------------------
void F3DStarter::ResetWindowName()
{
  f3d::window& window = this->Internals->Engine->getWindow();
  window.setWindowName(F3D::AppTitle).setIcon(F3DIcon, sizeof(F3DIcon));
}

//----------------------------------------------------------------------------
void F3DStarter::AddCommands()
{
  f3d::interactor& interactor = this->Internals->Engine->getInteractor();

  static const auto parse_optional_bool_flag =
    [](const std::vector<std::string>& args, std::string_view commandName, bool defaultValue)
  {
    if (args.empty())
    {
      return defaultValue;
    }
    if (args.size() != 1)
    {
      throw f3d::interactor::invalid_args_exception(std::string("Command: ") +
        std::string(commandName) + " takes at most 1 argument, got " + std::to_string(args.size()) +
        " arguments instead.");
    }
    return f3d::options::parse<bool>(args[0]);
  };

  interactor.addCommand("remove_file_groups",
    [this](const std::vector<std::string>&)
    {
      if (!this->Internals->AppOptions.NoRender)
      {
        this->Internals->Engine->getInteractor().stopAnimation();
      }
      this->Internals->FilesGroups.clear();
      this->LoadFileGroup(0, false, true);
      this->ResetWindowName();
    });

  interactor.addCommand("load_previous_file_group",
    [this](const std::vector<std::string>& args)
    {
      this->LoadRelativeFileGroup(
        -1, parse_optional_bool_flag(args, "load_previous_file_group", false));
    });

  interactor.addCommand("load_next_file_group",
    [this](const std::vector<std::string>& args)
    {
      this->LoadRelativeFileGroup(
        +1, parse_optional_bool_flag(args, "load_next_file_group", false));
    });

  interactor.addCommand("reload_current_file_group",
    [this](const std::vector<std::string>&) { this->LoadRelativeFileGroup(0, true, true); });

  interactor.addCommand("add_current_directories",
    [this](const std::vector<std::string>&)
    {
      if (!this->Internals->LoadedFiles.empty())
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
        index = this->AddFile(f3d::utils::collapsePath(file));
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
          index = this->AddFile(f3d::utils::collapsePath(file));
        }
      }
      if (index > -1)
      {
        this->LoadFileGroup(index);
      }
    });

#if F3D_MODULE_TINYFILEDIALOGS
  interactor.addCommand("open_file_dialog",
    [this](const std::vector<std::string>&)
    {
      std::vector<std::string> filters;
      for (const auto& info : f3d::engine::getReadersInfo())
      {
        for (const auto& ext : info.Extensions)
        {
          filters.push_back("*." + ext);
        }
      }

      std::vector<const char*> cstrings;
      cstrings.reserve(filters.size());
      for (const auto& filter : filters)
      {
        cstrings.push_back(filter.c_str());
      }

      const char* file = std::getenv("CTEST_OPEN_DIALOG_FILE");
      if (!file)
      {
        file = tinyfd_openFileDialog("Open File", nullptr, static_cast<int>(cstrings.size()),
          cstrings.data(), "Supported Files", false);
      }

      if (file)
      {
        int index = this->AddFile(file);
        if (index > -1)
        {
          this->LoadFileGroup(index);
        }
      }
    });
#endif
  interactor.addCommand("exit", [&](const std::vector<std::string>&) { interactor.stop(); });
}
