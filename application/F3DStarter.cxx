#include "F3DStarter.h"

#include "F3DConfig.h"
#include "F3DIcon.h"
#include "F3DNSDelegate.h"
#include "F3DOptionsParser.h"

#include "engine.h"
#include "interactor.h"
#include "log.h"
#include "options.h"
#include "window.h"

#include <cassert>
#include <filesystem>
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
    if (appOptions.CameraViewAngle != 0)
    {
      cam.setViewAngle(appOptions.CameraViewAngle);
    }
    cam.azimuth(appOptions.CameraAzimuthAngle)
      .elevation(appOptions.CameraElevationAngle)
      .setCurrentAsDefault();
  }

  F3DOptionsParser Parser;
  F3DAppOptions AppOptions;
  f3d::options DynamicOptions;
  f3d::options FileOptions;
  std::unique_ptr<f3d::engine> Engine;
  std::vector<fs::path> FilesList;
  int CurrentFileIndex = -1;
  bool LoadedFile = false;
  bool UpdateWithCommandLineParsing = true;
};

//----------------------------------------------------------------------------
F3DStarter::F3DStarter()
  : Internals(std::make_unique<F3DStarter::F3DInternals>())
{
  // Set option outside of command line and config file
  this->Internals->DynamicOptions.set(
    "ui.dropzone-info", "Drop a file to open it\nPress H to show cheatsheet");
}

//----------------------------------------------------------------------------
F3DStarter::~F3DStarter() = default;

//----------------------------------------------------------------------------
int F3DStarter::Start(int argc, char** argv)
{
  // Parse command line options
  std::vector<std::string> files;
  this->Internals->Parser.Initialize(argc, argv);
  this->Internals->Parser.GetOptions(
    this->Internals->AppOptions, this->Internals->DynamicOptions, files);

  // Set verbosity level early from command line
  if (this->Internals->AppOptions.Quiet)
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::QUIET);
  }
  else if (this->Internals->AppOptions.Verbose || this->Internals->AppOptions.NoRender)
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  }

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
    if (this->Internals->AppOptions.Quiet)
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::QUIET);
    }
    else if (this->Internals->AppOptions.Verbose || this->Internals->AppOptions.NoRender)
    {
      f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
    }
  }

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

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
        const auto loadFile = [this](int index, bool restoreCamera = false) -> bool
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

          this->Render();
          return true;
        };

        if (keySym == "Left")
        {
          return loadFile(-1);
        }
        else if (keySym == "Right")
        {
          return loadFile(+1);
        }
        else if (keySym == "Up")
        {
          return loadFile(0, true);
        }
        else if (keySym == "Down")
        {
          if (this->Internals->LoadedFile)
          {
            this->Internals->Engine->getInteractor().stopAnimation();
            this->AddFile(
              this->Internals->FilesList[static_cast<size_t>(this->Internals->CurrentFileIndex)]
                .parent_path(),
              true);
            return loadFile(0);
          }
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
          index = this->AddFile(fs::path(file));
        }
        if (index > -1)
        {
          this->LoadFile(index);
        }
        this->Render();
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

    // Render and compare with file if needed
    if (!this->Internals->AppOptions.Reference.empty())
    {
      if (!this->Internals->LoadedFile)
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
    }
    // Render to file if needed
    else if (!this->Internals->AppOptions.Output.empty())
    {
      if (!this->Internals->LoadedFile)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      f3d::image img = window.renderToImage(this->Internals->AppOptions.NoBackground);
      img.save(this->Internals->AppOptions.Output);
    }
    // Start interaction
    else
    {
#ifdef F3D_HEADLESS_BUILD
      f3d::log::error("This is a headless build of F3D, interactive rendering is not supported");
#else
      window.render();
      interactor.start();
#endif
    }
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void F3DStarter::LoadFile(int index, bool relativeIndex)
{
  if (this->Internals->LoadedFile)
  {
    // When loading a file after another, store the changed options
    // into the dynamic options and use these dynamic option as the default
    // for loading the next file while still applying file specific options on top of it

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
          f3d::log::warn(filePath, " is not a file of a supported file format\n");
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

  if (!this->Internals->LoadedFile)
  {
    // No file loaded, remove any previously loaded file
    loader.loadGeometry("", true);
  }

  this->Internals->Engine->getOptions().set("ui.dropzone", !this->Internals->LoadedFile);
  this->Internals->Engine->getOptions().set("ui.filename-info", filenameInfo);
}

//----------------------------------------------------------------------------
void F3DStarter::Render()
{
  this->Internals->Engine->getWindow().render();
}

//----------------------------------------------------------------------------
int F3DStarter::AddFile(const fs::path& path, bool quiet)
{
  auto tmpPath = fs::absolute(path);
  if (!fs::exists(tmpPath))
  {
    if (!quiet)
    {
      f3d::log::error("File ", tmpPath, " does not exist");
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
      this->Internals->FilesList.push_back(tmpPath);
      return static_cast<int>(this->Internals->FilesList.size()) - 1;
    }
    else
    {
      if (!quiet)
      {
        f3d::log::warn("File ", tmpPath, " has already been added");
      }
      return static_cast<int>(std::distance(this->Internals->FilesList.begin(), it));
    }
  }
}
