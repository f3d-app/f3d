#include "F3DStarter.h"

#include "F3DIcon.h"
#include "F3DOptions.h"

#include "F3DLog.h"
#include "F3DNSDelegate.h"
#include "f3d_interactor.h"
#include "f3d_options.h"
#include "f3d_windowNoRender.h"
#include "f3d_windowStandard.h"

class F3DStarter::F3DInternals
{
public:
  F3DInternals()
    : Loader(this->NewOptions)
  {
  }

  ~F3DInternals()
  {
    // TODO
    if (this->Window)
    {
      delete this->Window;
    }
  }

  F3DOptionsParser Parser;
  F3DOptions CommandLineOptions;
  F3DOptions Options;
  f3d::options NewOptions;
  f3d::loader Loader;
  f3d::interactor Interactor;
  f3d::window* Window = nullptr;
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
  this->Internals->Loader.addFile(path);
}

//----------------------------------------------------------------------------
int F3DStarter::Start(int argc, char** argv)
{
  // Parse command line options
  std::vector<std::string> files;
  this->Internals->Parser.Initialize(argc, argv);
  this->Internals->CommandLineOptions = this->Internals->Parser.GetOptionsFromCommandLine(files);

  // Respect quiet option
  F3DLog::SetQuiet(this->Internals->CommandLineOptions.Quiet);

  // Initialize the config file dictionary
  this->Internals->Parser.InitializeDictionaryFromConfigFile(
    this->Internals->CommandLineOptions.UserConfigFile);

#if __APPLE__
  // Initialize MacOS delegate
  F3DNSDelegate::InitializeDelegate(this);
#endif

  // TODO add f3d::noRenderWindow
  if (this->Internals->CommandLineOptions.NoRender)
  {
    this->Internals->Window = new f3d::windowNoRender();
    this->Internals->Loader.setWindow(this->Internals->Window);
  }
  else
  {
    // TODO Test this multiconfig behavior
    this->Internals->Interactor.setKeyPressCallBack(
      [this](int, std::string keySym) -> bool
      {
        if (keySym == "Left")
        {
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_PREVIOUS;
          this->LoadFile(load);
          this->Internals->Window->render();
          return true;
        }
        else if (keySym == "Right")
        {
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_NEXT;
          this->LoadFile(load);
          this->Internals->Window->render();
          return true;
        }
        return false;
      });

    this->Internals->Interactor.setDropFilesCallBack(
      [this](std::vector<std::string> filesVec) -> bool
      {
        for (std::string file : filesVec)
        {
          this->AddFile(file);
        }
        this->LoadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
        return true;
      });

    bool offscreen = !this->Internals->CommandLineOptions.Reference.empty() ||
      !this->Internals->CommandLineOptions.Output.empty();
    this->Internals->Window =
      new f3d::windowStandard(f3d::AppTitle, offscreen, F3DIcon, sizeof(F3DIcon));
    this->Internals->Loader.setWindow(this->Internals->Window);

    this->Internals->Loader.setInteractor(&this->Internals->Interactor);
  }

  // Add and load file
  this->Internals->Loader.addFiles(files);
  bool loaded = this->LoadFile();

  if (!this->Internals->CommandLineOptions.NoRender)
  {
    // Play recording if any
    if (!this->Internals->CommandLineOptions.InteractionTestPlayFile.empty())
    {
      if (!this->Internals->Interactor.playInteraction(this->Internals->CommandLineOptions.InteractionTestPlayFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Start recording if needed
    if (!this->Internals->CommandLineOptions.InteractionTestRecordFile.empty())
    {
      if (!this->Internals->Interactor.recordInteraction(this->Internals->CommandLineOptions.InteractionTestPlayFile))
      {
        return EXIT_FAILURE;
      }
    }

    // Render and compare with file if needed
    if (!this->Internals->CommandLineOptions.Reference.empty())
    {
      if (!loaded)
      {
        F3DLog::Print(F3DLog::Severity::Error, "No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!this->Internals->Window->renderAndCompareWithFile(this->Internals->CommandLineOptions.Reference,
      this->Internals->CommandLineOptions.RefThreshold, this->Internals->CommandLineOptions.NoBackground, this->Internals->CommandLineOptions.Output))
      {
        return EXIT_FAILURE;
      }
    }
    // Render to file if needed
    else if(!this->Internals->CommandLineOptions.Output.empty())
    {
      if (!loaded)
      {
        F3DLog::Print(F3DLog::Severity::Error, "No file loaded, no rendering performed");
        return EXIT_FAILURE;
      }

      if (!this->Internals->Window->renderToFile(this->Internals->CommandLineOptions.Output, this->Internals->CommandLineOptions.NoBackground))
      {
        return EXIT_FAILURE;
      }
    }
    // Start interaction
    else 
    {
      this->Internals->Interactor.start();
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
  this->Internals->Loader.getFileInfo(load, index, filePath, fileInfo);

  if (this->Internals->CommandLineOptions.DryRun)
  {
    // Use command line options
    F3DOptionsParser::ConvertToNewAPI(
      this->Internals->CommandLineOptions, &this->Internals->NewOptions);
  }
  else
  {
    // Recover options for the file to load
    F3DOptionsParser::ConvertToNewAPI(
      this->Internals->Parser.GetOptionsFromConfigFile(filePath), &this->Internals->NewOptions);
  }

  // With NoRender, force verbose
  if (this->Internals->CommandLineOptions.NoRender)
  {
    this->Internals->NewOptions.set("verbose", true);
  }

  // Load the file
  return this->Internals->Loader.loadFile(load);
}
