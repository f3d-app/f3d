#include "F3DStarter.h"

#include "F3DColorMapTools.h"
#include "F3DConfig.h"
#include "F3DIcon.h"
#include "F3DNSDelegate.h"
#include "F3DOptionsParser.h"
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

  void SetupCamera(const F3DAppOptions& appOptions)
  {
    f3d::camera& cam = this->Engine->getWindow().getCamera();
    if (appOptions.CameraPosition.size() == 3)
    {
      f3d::point3_t pos;
      std::copy_n(appOptions.CameraPosition.begin(), 3, pos.begin());
      cam.setPosition(pos);
    }
    if (appOptions.CameraFocalPoint.size() == 3)
    {
      f3d::point3_t foc;
      std::copy_n(appOptions.CameraFocalPoint.begin(), 3, foc.begin());
      cam.setFocalPoint(foc);
    }
    if (appOptions.CameraViewUp.size() == 3)
    {
      f3d::vector3_t up;
      std::copy_n(appOptions.CameraViewUp.begin(), 3, up.begin());
      cam.setViewUp(up);
    }
    if (appOptions.CameraViewAngle > 0)
    {
      cam.setViewAngle(appOptions.CameraViewAngle);
    }

    bool reset = false;
    double zoomFactor = 0.9;
    if (appOptions.CameraPosition.size() != 3 && appOptions.CameraDirection.size() == 3)
    {
      f3d::vector3_t dir;
      std::copy_n(appOptions.CameraDirection.begin(), 3, dir.begin());
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
    if (appOptions.CameraPosition.size() != 3 && appOptions.CameraZoomFactor > 0)
    {
      zoomFactor = appOptions.CameraZoomFactor;
      reset = true;
    }
    if (reset)
    {
      cam.resetToBounds(zoomFactor);
    }

    cam.azimuth(appOptions.CameraAzimuthAngle)
      .elevation(appOptions.CameraElevationAngle)
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
  std::filesystem::path applyFilenameTemplate(const std::string& templateString)
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

      std::vector<std::pair<std::string, bool> > fragments;
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
      if (!std::filesystem::exists(candidate))
      {
        return { candidate };
      }
    }
    throw std::runtime_error("could not find available unique filename after " +
      std::to_string(maxNumberingAttempts) + " attempts");
  }

  F3DOptionsParser Parser;
  F3DAppOptions AppOptions;
  f3d::options DynamicOptions;
  f3d::options FileOptions;
  std::unique_ptr<f3d::engine> Engine;
  std::vector<fs::path> FilesList;
  dmon_watch_id FolderWatchId;
  bool LoadedFile = false;
  bool UpdateWithCommandLineParsing = true;

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
  // Set option outside of command line and config file
  this->Internals->DynamicOptions.set(
    "ui.dropzone-info", "Drop a file or HDRI to load it\nPress H to show cheatsheet");

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
  // Parse command line options
  std::vector<std::string> files;
  this->Internals->Parser.Initialize(argc, argv);
  this->Internals->Parser.GetOptions(
    this->Internals->AppOptions, this->Internals->DynamicOptions, files);

  const bool renderToStdout = this->Internals->AppOptions.Output == "-";

  // Set verbosity level early from command line
  F3DInternals::SetVerboseLevel(this->Internals->AppOptions.VerboseLevel, renderToStdout);

  if (renderToStdout)
  {
    f3d::log::info("Output image will be saved to stdout, all log types including debug and info "
                   "levels are redirected to stderr");
  }

  f3d::log::debug("========== Initializing ==========");

  // Load plugins from the app options
  this->Internals->Parser.LoadPlugins(this->Internals->AppOptions);

  // Read config file if needed
  if (!this->Internals->AppOptions.DryRun)
  {
    // Initialize the config file dictionary
    this->Internals->Parser.InitializeDictionaryFromConfigFile(
      this->Internals->AppOptions.UserConfigFile);

    // Parse command line options with config file, global section only
    this->Internals->Parser.GetOptions(
      this->Internals->AppOptions, this->Internals->DynamicOptions, files);

    // Set verbosity level again if it was defined in the configuration file global block
    F3DInternals::SetVerboseLevel(this->Internals->AppOptions.VerboseLevel, renderToStdout);
  }

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  f3d::log::debug("========== Configuring engine ==========");

  if (this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine = std::make_unique<f3d::engine>(f3d::window::Type::NONE);
  }
  else
  {
    bool offscreen =
      !this->Internals->AppOptions.Reference.empty() || !this->Internals->AppOptions.Output.empty();
    this->Internals->Engine = std::make_unique<f3d::engine>(
      offscreen ? f3d::window::Type::NATIVE_OFFSCREEN : f3d::window::Type::NATIVE);

    f3d::window& window = this->Internals->Engine->getWindow();
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
            this->Internals->Engine->getInteractor().stopAnimation();
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

        return false;
      });

    interactor.setDropFilesCallBack(
      [this](const std::vector<std::string>& filesVec) -> bool
      {
        this->Internals->Engine->getInteractor().stopAnimation();
        int index = -1;
        for (const std::string& file : filesVec)
        {
          if (F3DInternals::HasHDRIExtension(file))
          {
            // TODO: add a image::canRead

            // Load the file as an HDRI instead of adding it.
            this->Internals->Engine->getOptions().set("render.hdri.file", file);
            this->Internals->Engine->getOptions().set("render.hdri.ambient", true);
            this->Internals->Engine->getOptions().set("render.background.skybox", true);

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
    window
      .setSize(this->Internals->AppOptions.Resolution[0], this->Internals->AppOptions.Resolution[1])
      .setWindowName(F3D::AppTitle)
      .setIcon(F3DIcon, sizeof(F3DIcon));

    if (this->Internals->AppOptions.Position.size() == 2)
    {
      window.setPosition(
        this->Internals->AppOptions.Position[0], this->Internals->AppOptions.Position[1]);
    }
#ifdef __APPLE__
    else
    {
      // The default position (50, 50) in VTK on MacOS is not a great fit for F3D as it can be
      // partially hidden because the position correspond to the upper left corner and the Y
      // position is defined from the bottom of the screen. Position it somewhere it makes sense.
      window.setPosition(100, 800);
    }
#endif
  }

  // Parse colormap
  if (!this->Internals->AppOptions.ColorMapFile.empty())
  {
    std::string fullPath = F3DColorMapTools::Find(this->Internals->AppOptions.ColorMapFile);

    if (!fullPath.empty())
    {
      this->Internals->Engine->getOptions().set(
        "model.scivis.colormap", F3DColorMapTools::Read(fullPath));
    }
    else
    {
      f3d::log::error("Cannot find the colormap ", this->Internals->AppOptions.ColorMapFile);
      this->Internals->Engine->getOptions().set("model.scivis.colormap", std::vector<double>{});
    }
  }

  f3d::log::debug("Engine configured");

  // Add all files
  for (auto& file : files)
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
    if (!this->Internals->AppOptions.InteractionTestPlayFile.empty())
    {
      // For better testing, render once before the interaction
      window.render();
      if (!interactor.playInteraction(this->Internals->AppOptions.InteractionTestPlayFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Start recording if needed
    if (!this->Internals->AppOptions.InteractionTestRecordFile.empty())
    {
      if (!interactor.recordInteraction(this->Internals->AppOptions.InteractionTestRecordFile))
      {
        return EXIT_FAILURE;
      }
    }

    char* noDataForceRender = std::getenv("CTEST_F3D_NO_DATA_FORCE_RENDER");

    // Render and compare with file if needed
    if (!this->Internals->AppOptions.Reference.empty())
    {
      if (!this->Internals->LoadedFile && !noDataForceRender)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!fs::exists(this->Internals->AppOptions.Reference))
      {
        if (this->Internals->AppOptions.Output.empty())
        {
          f3d::log::error("Reference image ", this->Internals->AppOptions.Reference,
            " does not exist, use the output option to output current rendering into an image "
            "file.\n");
        }
        else
        {
          window.renderToImage(this->Internals->AppOptions.NoBackground)
            .save(this->Internals->AppOptions.Output);

          f3d::log::error("Reference image " + this->Internals->AppOptions.Reference +
            " does not exist, current rendering has been outputted to " +
            this->Internals->AppOptions.Output + ".\n");
        }
        return EXIT_FAILURE;
      }

      f3d::image img = window.renderToImage(this->Internals->AppOptions.NoBackground);
      f3d::image ref(this->Internals->AppOptions.Reference);
      f3d::image diff;
      double error;
      if (!img.compare(ref, this->Internals->AppOptions.RefThreshold, diff, error))
      {
        if (this->Internals->AppOptions.Output.empty())
        {
          f3d::log::error("Use the --output option to be able to output current rendering and diff "
                          "images into files.\n");
        }
        else
        {
          f3d::log::error("Current rendering difference with reference image: ", error,
            " is higher than the threshold of ", this->Internals->AppOptions.RefThreshold, ".\n");

          img.save(this->Internals->AppOptions.Output);
          diff.save(
            fs::path(this->Internals->AppOptions.Output).replace_extension(".diff.png").string());
        }
        return EXIT_FAILURE;
      }
      else
      {
        f3d::log::info("Image comparison success with an error difference of: ", error);
      }

      if (this->Internals->FilesList.size() > 1)
      {
        f3d::log::warn("Image comparison was performed using a single 3D file, other provided "
                       "3D files were ignored.");
      }
    }
    // Render to file if needed
    else if (!this->Internals->AppOptions.Output.empty())
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
        std::filesystem::path path =
          this->Internals->applyFilenameTemplate(this->Internals->AppOptions.Output);
        img.save(path.string());
        f3d::log::debug("Output image saved to ", path);
      }

      if (this->Internals->FilesList.size() > 1)
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
  f3d::log::debug("========== Loading 3D file ==========");
  // When loading a file, store any changed options
  // into the dynamic options and use these dynamic option as the default
  // for loading the file while still applying file specific options on top of it

  // Recover previous options from the engine
  const f3d::options& previousOptions = this->Internals->Engine->getOptions();

  // Detect changed options and apply the change to the dynamic options
  // options names are shared between options instance
  std::vector<std::string> optionNames = this->Internals->DynamicOptions.getNames();
  for (const auto& name : optionNames)
  {
    if (!previousOptions.isSame(this->Internals->FileOptions, name))
    {
      this->Internals->DynamicOptions.copy(previousOptions, name);
    }
  }

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

    // Copy dynamic options into files options to get the global config
    this->Internals->FileOptions = this->Internals->DynamicOptions;
    this->Internals->Engine->setOptions(this->Internals->FileOptions);
  }

  if (this->Internals->CurrentFileIndex >= 0)
  {
    if (this->Internals->AppOptions.GroupGeometries)
    {
      // Group geometries mode, consider the first file configuration file only
      this->Internals->CurrentFileIndex = 0;
      filePath = this->Internals->FilesList[static_cast<size_t>(this->Internals->CurrentFileIndex)];
    }

    // Update options for the file to load, using dynamic options as default
    this->Internals->FileOptions = this->Internals->DynamicOptions;
    F3DAppOptions fileAppOptions = this->Internals->AppOptions;
    this->Internals->Parser.UpdateOptions(filePath.string(), fileAppOptions,
      this->Internals->FileOptions, this->Internals->UpdateWithCommandLineParsing);
    this->Internals->UpdateWithCommandLineParsing = false; // this is done only once
    this->Internals->Engine->setOptions(this->Internals->FileOptions);

    this->Internals->LoadedFile = false;

    // Load any new plugins the updated app options
    this->Internals->Parser.LoadPlugins(fileAppOptions);

    // Check the size of the file before loading it
    // Not considered in the context of GroupGeometries
    static constexpr int BYTES_IN_MIB = 1048576;
    if (fileAppOptions.MaxSize >= 0.0 &&
      fs::file_size(filePath) > static_cast<std::uintmax_t>(fileAppOptions.MaxSize * BYTES_IN_MIB))
    {
      f3d::log::info("No file loaded, file is bigger than max size");
    }
    else
    {
      try
      {
        if (loader.hasSceneReader(filePath.string()) && !fileAppOptions.GeometryOnly &&
          !fileAppOptions.GroupGeometries)
        {
          loader.loadScene(filePath.string());
          this->Internals->LoadedFile = true;
        }
        else if (loader.hasGeometryReader(filePath.string()))
        {
          // In GroupGeometries, just load all the files from the list
          if (fileAppOptions.GroupGeometries)
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

      if (!this->Internals->AppOptions.NoRender)
      {
        // Setup the camera according to options
        this->Internals->SetupCamera(fileAppOptions);

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

  this->Internals->Engine->getOptions().set("ui.dropzone", !this->Internals->LoadedFile);
  this->Internals->Engine->getOptions().set("ui.filename-info", filenameInfo);
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
void F3DStarter::SaveScreenshot(const std::string& filenameTemplate)
{

  const auto getScreenshotDir = []()
  {
    for (const char* const& candidate : { "XDG_PICTURES_DIR", "HOME", "USERPROFILE" })
    {
      char* val = std::getenv(candidate);
      if (val != nullptr)
      {
        std::filesystem::path path(val);
        if (std::filesystem::is_directory(path))
        {
          return path;
        }
      }
    }

    return std::filesystem::current_path();
  };

  std::filesystem::path pathTemplate = std::filesystem::path(filenameTemplate).make_preferred();
  std::filesystem::path fullPathTemplate =
    pathTemplate.is_absolute() ? pathTemplate : getScreenshotDir() / pathTemplate;
  std::filesystem::path path = this->Internals->applyFilenameTemplate(fullPathTemplate.string());

  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  f3d::log::info("saving screenshot to " + path.string());

  f3d::image img =
    this->Internals->Engine->getWindow().renderToImage(this->Internals->AppOptions.NoBackground);
  this->Internals->addOutputImageMetadata(img);
  img.save(path.string(), f3d::image::SaveFormat::PNG);

  f3d::options& options = this->Internals->Engine->getOptions();
  const std::string light_intensity_key = "render.light.intensity";
  const double intensity = options.getAsDouble(light_intensity_key);
  options.set(light_intensity_key, intensity * 5);
  this->Render();
  options.set(light_intensity_key, intensity);
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
  this->Internals->Engine->getInteractor().stopAnimation();

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
