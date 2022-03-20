#include "F3DStarter.h"

#include "F3DIcon.h"
#include "F3DOptions.h"

#include "F3DNSDelegate.h"
#include "f3d_engine.h"
#include "f3d_interactor.h"
#include "f3d_log.h"
#include "f3d_options.h"
#include "f3d_windowNoRender.h"
#include "f3d_windowStandard.h"

class F3DStarter::F3DInternals
{
public:
  F3DInternals() = default;

  F3DOptionsParser Parser;
  F3DOptions CommandLineOptions;
  F3DOptions Options;
  std::unique_ptr<f3d::engine> Engine;
};

//----------------------------------------------------------------------------
F3DStarter::F3DStarter()
  : Internals(new F3DStarter::F3DInternals)
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
  this->Internals->CommandLineOptions = this->Internals->Parser.GetOptionsFromCommandLine(files);

  // Respect quiet option
  f3d::log::setQuiet(this->Internals->CommandLineOptions.Quiet);

  // Initialize the config file dictionary
  this->Internals->Parser.InitializeDictionaryFromConfigFile(
    this->Internals->CommandLineOptions.UserConfigFile);

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  if (this->Internals->CommandLineOptions.NoRender)
  {
    this->Internals->Engine =
      std::make_unique<f3d::engine>(f3d::engine::WindowTypeEnum::WINDOW_NO_RENDER, false);
  }
  else
  {
    // TODO Test this multiconfig behavior
    bool offscreen = !this->Internals->CommandLineOptions.Reference.empty() ||
      !this->Internals->CommandLineOptions.Output.empty();

    this->Internals->Engine =
      std::make_unique<f3d::engine>(f3d::engine::WindowTypeEnum::WINDOW_NO_RENDER, offscreen);

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

    this->Internals->Engine->getInteractor().setDropFilesCallBack(
      [this](std::vector<std::string> filesVec) -> bool
      {
        this->Internals->Engine->getInteractor().stopAnimation();
        for (std::string file : filesVec)
        {
          this->AddFile(file);
        }
        this->LoadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
        return true;
      });
    this->Internals->Engine->getWindow().setIcon(F3DIcon, sizeof(F3DIcon));
  }

  // Add and load file
  this->Internals->Engine->getLoader().addFiles(files);
  bool loaded = this->LoadFile();

  if (!this->Internals->CommandLineOptions.NoRender)
  {
    f3d::interactor& interactor = this->Internals->Engine->getInteractor();

    // Play recording if any
    if (!this->Internals->CommandLineOptions.InteractionTestPlayFile.empty())
    {
      if (!interactor.playInteraction(this->Internals->CommandLineOptions.InteractionTestPlayFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Start recording if needed
    if (!this->Internals->CommandLineOptions.InteractionTestRecordFile.empty())
    {
      if (!interactor.recordInteraction(
            this->Internals->CommandLineOptions.InteractionTestRecordFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Render and compare with file if needed
    if (!this->Internals->CommandLineOptions.Reference.empty())
    {
      if (!loaded)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!this->Internals->Engine->getWindow().renderAndCompareWithFile(
            this->Internals->CommandLineOptions.Reference,
            this->Internals->CommandLineOptions.RefThreshold,
            this->Internals->CommandLineOptions.NoBackground,
            this->Internals->CommandLineOptions.Output))
      {
        return EXIT_FAILURE;
      }
    }
    // Render to file if needed
    else if (!this->Internals->CommandLineOptions.Output.empty())
    {
      if (!loaded)
      {
        f3d::log::error("No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!this->Internals->Engine->getWindow().renderToFile(
            this->Internals->CommandLineOptions.Output,
            this->Internals->CommandLineOptions.NoBackground))
      {
        return EXIT_FAILURE;
      }
    }
    // Start interaction
    else
    {
      this->Internals->Engine->getInteractor().start();
    }
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool F3DStarter::LoadFile(f3d::loader::LoadFileEnum load)
{
  // Recover info about the file to be loaded
  int index;
  std::string filePath, fileInfo;
  this->Internals->Engine->getLoader().getFileInfo(load, index, filePath, fileInfo);

  f3d::options& options = this->Internals->Engine->getOptions();
  if (this->Internals->CommandLineOptions.DryRun)
  {
    // Use command line options
    F3DOptionsParser::ConvertToNewAPI(this->Internals->CommandLineOptions, &options);
  }
  else
  {
    // Recover options for the file to load
    F3DOptionsParser::ConvertToNewAPI(
      this->Internals->Parser.GetOptionsFromConfigFile(filePath), &options);
  }

  // With NoRender, force verbose
  if (this->Internals->CommandLineOptions.NoRender)
  {
    options.set("verbose", true);
  }

  // Load the file
  return this->Internals->Engine->getLoader().loadFile(load);
}
