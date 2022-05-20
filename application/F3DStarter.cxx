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

class F3DStarter::F3DInternals
{
public:
  F3DInternals() = default;

  F3DOptionsParser Parser;
  F3DAppOptions AppOptions;
  std::unique_ptr<f3d::engine> Engine;
  bool LoadedFile = false;
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
  f3d::options commandLineOptions;
  this->Internals->Parser.GetOptionsFromCommandLine(
    this->Internals->AppOptions, commandLineOptions, files);

  // Respect quiet option
  f3d::log::setQuiet(commandLineOptions.getAsBool("quiet"));

  // Initialize the config file dictionary
  this->Internals->Parser.InitializeDictionaryFromConfigFile(
    this->Internals->AppOptions.UserConfigFile);

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  if (this->Internals->AppOptions.NoRender)
  {
    this->Internals->Engine = std::make_unique<f3d::engine>(f3d::engine::FLAGS_NONE);
  }
  else
  {
    // TODO Test this multiconfig behavior
    bool offscreen =
      !this->Internals->AppOptions.Reference.empty() || !this->Internals->AppOptions.Output.empty();

    f3d::engine::flags_t flags = f3d::engine::CREATE_WINDOW | f3d::engine::CREATE_INTERACTOR |
      (offscreen ? f3d::engine::WINDOW_OFFSCREEN : f3d::engine::FLAGS_NONE);
    this->Internals->Engine = std::make_unique<f3d::engine>(flags);
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
        this->Internals->Engine->getWindow().render();
        return true;
      });
    this->Internals->Engine->getWindow().setWindowName(F3D::AppTitle);
    this->Internals->Engine->getWindow().setIcon(F3DIcon, sizeof(F3DIcon));
  }

  // Move command line options into the actual engine options
  this->Internals->Engine->setOptions(commandLineOptions);

  // Add and load file
  this->Internals->Engine->getLoader().addFiles(files);
  this->LoadFile();

  if (!this->Internals->AppOptions.NoRender)
  {
    f3d::interactor& interactor = this->Internals->Engine->getInteractor();

    // Play recording if any
    if (!this->Internals->AppOptions.InteractionTestPlayFile.empty())
    {
      // For better testing, render once before the interaction
      this->Internals->Engine->getWindow().render();
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

      if (!this->Internals->Engine->getWindow().renderAndCompareWithFile(
            this->Internals->AppOptions.Reference, this->Internals->AppOptions.RefThreshold,
            this->Internals->AppOptions.NoBackground, this->Internals->AppOptions.Output))
      {
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

      if (!this->Internals->Engine->getWindow().renderToFile(
            this->Internals->AppOptions.Output, this->Internals->AppOptions.NoBackground))
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
void F3DStarter::LoadFile(f3d::loader::LoadFileEnum load)
{
  // Recover info about the file to be loaded
  int index;
  std::string filePath, fileInfo;
  this->Internals->Engine->getLoader().getFileInfo(load, index, filePath, fileInfo);

  f3d::options& options = this->Internals->Engine->getOptions();
  if (!this->Internals->AppOptions.DryRun)
  {
    // Recover options for the file to load
    this->Internals->Parser.GetOptionsFromConfigFile(filePath, options);
  }

  // With NoRender, force verbose
  if (this->Internals->AppOptions.NoRender)
  {
    options.set("verbose", true);
  }

  // Load the file
  this->Internals->LoadedFile = this->Internals->Engine->getLoader().loadFile(load);
  return;
}
