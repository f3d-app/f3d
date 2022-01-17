#include "F3DStarter.h"

#include "F3DIcon.h"
#include "F3DOptions.h"

#include "F3DLog.h"
#include "F3DNSDelegate.h"
#include "f3d_options.h"

// TODO to remove when possible
#include "vtkF3DInteractorStyle.h"

// TODO to remove when possible
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkStringArray.h>

class F3DStarter::F3DInternals
{
public:
  F3DOptionsParser Parser;
  F3DOptions CommandLineOptions;
  F3DOptions Options;
  f3d::options NewOptions;
  F3DLoader Loader;
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
  this->Internals->Loader.AddFile(path);
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

  if (!this->Internals->CommandLineOptions.NoRender)
  {
    // TODO For now with an initialize
    bool offscreen = !this->Internals->CommandLineOptions.Reference.empty() ||
      !this->Internals->CommandLineOptions.Output.empty();
    this->Internals->Loader.InitializeRendering(f3d::AppTitle, offscreen, F3DIcon, sizeof(F3DIcon));

    // TODO introduce clean f3d::interactor
    vtkF3DInteractorStyle* style = this->Internals->Loader.GetInteractorStyle();

    // Setup the observers for the interactor style events
    vtkNew<vtkCallbackCommand> newFilesCallback;
    newFilesCallback->SetClientData(this);
    newFilesCallback->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData)
      {
        F3DStarter* starter = static_cast<F3DStarter*>(clientData);
        vtkStringArray* filesArr = static_cast<vtkStringArray*>(callData);
        for (int i = 0; i < filesArr->GetNumberOfTuples(); i++)
        {
          starter->AddFile(filesArr->GetValue(i));
        }
        starter->LoadFile(F3DLoader::LoadFileEnum::LOAD_LAST);
      });
    style->AddObserver(vtkF3DInteractorStyle::NewFilesEvent, newFilesCallback);

    vtkNew<vtkCallbackCommand> loadFileCallback;
    loadFileCallback->SetClientData(this);
    loadFileCallback->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData)
      {
        F3DStarter* starter = static_cast<F3DStarter*>(clientData);
        F3DLoader::LoadFileEnum* load = static_cast<F3DLoader::LoadFileEnum*>(callData);
        starter->LoadFile(*load);
      });
    style->AddObserver(vtkF3DInteractorStyle::LoadFileEvent, loadFileCallback);

    vtkNew<vtkCallbackCommand> toggleAnimationCallback;
    toggleAnimationCallback->SetClientData(this);
    toggleAnimationCallback->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void*)
      {
        F3DStarter* starter = static_cast<F3DStarter*>(clientData);
        starter->Internals->Loader.ToggleAnimation();
      });
    style->AddObserver(vtkF3DInteractorStyle::ToggleAnimationEvent, toggleAnimationCallback);
  }

  // Add and load file
  this->Internals->Loader.AddFiles(files);
  this->LoadFile();

  if (!this->Internals->CommandLineOptions.NoRender)
  {
    // Start rendering and interaction
    if (!this->Internals->Loader.Start())
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool F3DStarter::LoadFile(F3DLoader::LoadFileEnum load)
{
  // Recover info about the file to be loaded
  int index;
  std::string filePath, fileInfo;
  this->Internals->Loader.GetNextFile(load, index, filePath, fileInfo);

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
  return this->Internals->Loader.LoadFile(load, this->Internals->NewOptions);
}
