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

#include <filesystem>

class F3DStarter::F3DInternals
{
public:
  F3DInternals() = default;

  void SetupCamera(const F3DAppOptions& appOptions)
  {
    f3d::camera& cam = this->Engine->getWindow().getCamera();
    if (appOptions.CameraPosition.size() == 3)
    {
      f3d::camera::vector3_t pos;
      std::copy_n(appOptions.CameraPosition.begin(), 3, pos.begin());
      cam.setPosition(pos);
    }
    if (appOptions.CameraFocalPoint.size() == 3)
    {
      f3d::camera::vector3_t foc;
      std::copy_n(appOptions.CameraFocalPoint.begin(), 3, foc.begin());
      cam.setFocalPoint(foc);
    }
    if (appOptions.CameraViewUp.size() == 3)
    {
      f3d::camera::vector3_t up;
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
  std::unique_ptr<f3d::engine> Engine;
  bool LoadedFile = false;
};

//----------------------------------------------------------------------------
F3DStarter::F3DStarter()
  : Internals(std::make_unique<F3DStarter::F3DInternals>())
{
}

//----------------------------------------------------------------------------
F3DStarter::~F3DStarter() = default;

//----------------------------------------------------------------------------
void F3DStarter::AddFile(const std::string& path)
{
  this->Internals->Engine->getLoader().addFile(path);
}

//----------------------------------------------------------------------------
int F3DStarter::Start(int argc, char** argv)
{
  // Parse command line options
  std::vector<std::string> files;
  this->Internals->Parser.Initialize(argc, argv);
  f3d::options commandLineOptions;
  this->Internals->Parser.GetOptionsFromCommandLine(
    this->Internals->AppOptions, commandLineOptions, files);

  // Set verbosity level
  if (this->Internals->AppOptions.Quiet)
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::QUIET);
  }
  else if (this->Internals->AppOptions.Verbose || this->Internals->AppOptions.NoRender)
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  }

  // Initialize the config file dictionary
  this->Internals->Parser.InitializeDictionaryFromConfigFile(
    this->Internals->AppOptions.UserConfigFile);

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
      [this](int, std::string keySym) -> bool
      {
        if (keySym == "Left")
        {
          this->Internals->Engine->getInteractor().stopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_PREVIOUS;
          this->LoadFile(load);
          this->Internals->Engine->getWindow().render();
          return true;
        }
        else if (keySym == "Right")
        {
          this->Internals->Engine->getInteractor().stopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_NEXT;
          this->LoadFile(load);
          this->Internals->Engine->getWindow().render();
          return true;
        }
        return false;
      });

    interactor.setDropFilesCallBack(
      [this](std::vector<std::string> filesVec) -> bool
      {
        this->Internals->Engine->getInteractor().stopAnimation();
        for (std::string file : filesVec)
        {
          this->AddFile(file);
        }
        this->LoadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
        this->Internals->Engine->getWindow().render();
        return true;
      });
    window.setSize(
      this->Internals->AppOptions.Resolution[0], this->Internals->AppOptions.Resolution[1]);
    window.setWindowName(F3D::AppTitle);
    window.setIcon(F3DIcon, sizeof(F3DIcon));
  }

  // Move command line options into the actual engine options
  this->Internals->Engine->setOptions(commandLineOptions);

  // Add and load file
  this->Internals->Engine->getLoader().addFiles(files);
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

      if (!std::filesystem::exists(this->Internals->AppOptions.Reference))
      {
        if (this->Internals->AppOptions.Output.empty())
        {
          f3d::log::error("Reference image ", this->Internals->AppOptions.Reference,
            " does not exists, use the output option to output current rendering into an image "
            "file.\n");
        }
        else
        {
          window.renderToImage(this->Internals->AppOptions.NoBackground)
            .save(this->Internals->AppOptions.Output);

          f3d::log::error("Reference image " + this->Internals->AppOptions.Reference +
            " does not exists, current rendering has been outputted to " +
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
          diff.save(std::filesystem::path(this->Internals->AppOptions.Output)
                      .replace_extension(".diff.png")
                      .string());
        }
        return EXIT_FAILURE;
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
      interactor.start();
    }
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void F3DStarter::LoadFile(f3d::loader::LoadFileEnum load)
{
  // Recover info about the file to be loaded
  int index;
  std::string filePath, fileInfo;
  this->Internals->Engine->getLoader().getFileInfo(load, index, filePath, fileInfo);

  F3DAppOptions configFileAppOptions;
  if (!this->Internals->AppOptions.DryRun)
  {
    // Recover options for the file to load
    f3d::options configFileOptions;
    this->Internals->Parser.GetOptionsFromConfigFile(
      filePath, configFileAppOptions, configFileOptions);
    this->Internals->Engine->setOptions(std::move(configFileOptions));
  }

  // Load the file
  this->Internals->LoadedFile = this->Internals->Engine->getLoader().loadFile(load);

  if (!this->Internals->AppOptions.NoRender)
  {
    // Setup the camera according to options
    this->Internals->SetupCamera(
      this->Internals->AppOptions.DryRun ? this->Internals->AppOptions : configFileAppOptions);
  }

  return;
}
