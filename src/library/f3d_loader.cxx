#include "f3d_loader.h"

#include "F3DAnimationManager.h"
#include "F3DLog.h"
#include "F3DOffscreenRender.h"
#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"
#include "f3d_interactor.h"
#include "f3d_options.h"
#include "f3d_window.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DObjectFactory.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkLogger.h>
#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPointGaussianMapper.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkRenderWindow.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <algorithm>

namespace
{
typedef struct ProgressDataStruct
{
  vtkTimerLog* timer;
  vtkProgressBarWidget* widget;
} ProgressDataStruct;

vtkSmartPointer<vtkImporter> GetImporter(const f3d::options& options, const std::string& fileName)
{
  if (!options.get<bool>("geometry-only"))
  {
    // Try to find the first compatible reader with scene reading capabilities
    F3DReader* reader = F3DReaderFactory::GetReader(fileName);
    if (reader)
    {
      vtkSmartPointer<vtkImporter> importer = reader->CreateSceneReader(fileName);
      if (importer)
      {
        return importer;
      }
    }
  }

  // Use the generic importer and check if it can process the file
  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetFileName(fileName.c_str());
  importer->SetOptions(&options);
  if (!importer->CanReadFile())
  {
    return nullptr;
  }
  return importer;
}

void CreateProgressRepresentationAndCallback(
  ProgressDataStruct* data, vtkImporter* importer, f3d::interactor* interactor)
{
  vtkNew<vtkCallbackCommand> progressCallback;
  progressCallback->SetClientData(data);
  progressCallback->SetCallback(
    [](vtkObject*, unsigned long, void* clientData, void* callData)
    {
      auto progressData = static_cast<ProgressDataStruct*>(clientData);
      progressData->timer->StopTimer();
      vtkProgressBarWidget* widget = progressData->widget;
      // Only show and render the progress bar if loading takes more than 0.15 seconds
      if (progressData->timer->GetElapsedTime() > 0.15)
      {
        widget->On();
        vtkProgressBarRepresentation* rep =
          vtkProgressBarRepresentation::SafeDownCast(widget->GetRepresentation());
        rep->SetProgressRate(*static_cast<double*>(callData));
        widget->Render();
      }
    });
  importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);

  interactor->SetInteractorOn(data->widget);

  vtkProgressBarRepresentation* progressRep =
    vtkProgressBarRepresentation::SafeDownCast(data->widget->GetRepresentation());
  progressRep->SetProgressRate(0.0);
  progressRep->ProportionalResizeOff();
  progressRep->SetPosition(0.0, 0.0);
  progressRep->SetPosition2(1.0, 0.0);
  progressRep->SetMinimumSize(0, 5);
  progressRep->SetProgressBarColor(1, 1, 1);
  progressRep->DrawBackgroundOff();
  progressRep->DragableOff();
  progressRep->SetShowBorderToOff();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201027)
  progressRep->DrawFrameOff();
  progressRep->SetPadding(0.0, 0.0);
#endif
  data->timer->StartTimer();
}

void DisplayImporterDescription(vtkImporter* importer)
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210228)
  vtkIdType availCameras = importer->GetNumberOfCameras();
  if (availCameras <= 0)
  {
    F3DLog::Print(F3DLog::Severity::Info, "No camera available in this file");
  }
  else
  {
    F3DLog::Print(F3DLog::Severity::Info, "Camera(s) available in this file are:");
  }
  for (int i = 0; i < availCameras; i++)
  {
    F3DLog::Print(F3DLog::Severity::Info, i, ": ", importer->GetCameraName(i));
  }
  F3DLog::Print(F3DLog::Severity::Info, "\n");
#endif
  F3DLog::Print(F3DLog::Severity::Info, importer->GetOutputsDescription());
}
}

namespace f3d
{
class loader::F3DInternals
{
public:
  F3DInternals(const options& options)
    : Options(options)
  {
  }
  std::vector<std::string> FilesList;
  int CurrentFileIndex = 0;
  bool LoadedFile = false;
  const options& Options;
  interactor* Interactor = nullptr;
  window* Window = nullptr;
  F3DAnimationManager AnimationManager;
  vtkSmartPointer<vtkImporter> Importer;
  F3DReaderInstantiator ReaderInstantiator;
};

//----------------------------------------------------------------------------
loader::loader(const options& options)
  : Internals(new loader::F3DInternals(options))
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // Disable vtkLogger in case VTK was built with log support
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200701)
  vtkLogger::SetInternalVerbosityLevel(vtkLogger::VERBOSITY_OFF);
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  // Make sure to initialize the output window
  // after the object factory and before the first usage.
  F3DLog::SetQuiet(false);
}

//----------------------------------------------------------------------------
loader::~loader() = default;

//----------------------------------------------------------------------------
void loader::toggleAnimation()
{
  this->Internals->AnimationManager.ToggleAnimation();
}

//----------------------------------------------------------------------------
void loader::addFiles(const std::vector<std::string>& files)
{
  for (auto& file : files)
  {
    this->addFile(file);
  }
}

//----------------------------------------------------------------------------
void loader::addFile(const std::string& path, bool recursive)
{
  if (path.empty())
  {
    return;
  }

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);
  if (!vtksys::SystemTools::FileExists(fullPath))
  {
    F3DLog::Print(F3DLog::Severity::Error, "File ", fullPath, " does not exist");
    return;
  }

  if (vtksys::SystemTools::FileIsDirectory(fullPath))
  {
    if (recursive)
    {
      vtksys::Directory dir;
      dir.Load(fullPath);
      std::set<std::string> sortedFiles;

      // Sorting is necessary as KWSys can provide unsorted files
      for (unsigned long i = 0; i < dir.GetNumberOfFiles(); i++)
      {
        std::string currentFile = dir.GetFile(i);
        if (currentFile != "." && currentFile != "..")
        {
          sortedFiles.insert(currentFile);
        }
      }
      for (std::string currentFile : sortedFiles)
      {
        this->addFile(vtksys::SystemTools::JoinPath({ "", fullPath, currentFile }), false);
      }
    }
  }
  else
  {
    auto it =
      std::find(this->Internals->FilesList.begin(), this->Internals->FilesList.end(), fullPath);

    if (it == this->Internals->FilesList.end())
    {
      this->Internals->FilesList.push_back(fullPath);
    }
  }
}

//----------------------------------------------------------------------------
void loader::getFileInfo(
  LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const
{
  int size = static_cast<int>(this->Internals->FilesList.size());
  if (size > 0)
  {
    int addToIndex = 0;
    switch (load)
    {
      case loader::LoadFileEnum::LOAD_FIRST:
        nextFileIndex = 0;
        break;
      case loader::LoadFileEnum::LOAD_LAST:
        nextFileIndex = size - 1;
        break;
      case loader::LoadFileEnum::LOAD_PREVIOUS:
        addToIndex = -1;
        break;
      case loader::LoadFileEnum::LOAD_NEXT:
        addToIndex = 1;
        break;
      case loader::LoadFileEnum::LOAD_CURRENT:
      default:
        break;
    }

    // Compute the correct file index
    nextFileIndex = (this->Internals->CurrentFileIndex + addToIndex) % size;
    nextFileIndex = nextFileIndex < 0 ? nextFileIndex + size : nextFileIndex;

    filePath = this->Internals->FilesList[nextFileIndex];
    fileInfo = "(" + std::to_string(nextFileIndex + 1) + "/" + std::to_string(size) + ") " +
      vtksys::SystemTools::GetFilenameName(filePath);
  }
  else
  {
    nextFileIndex = -1;
  }
}

//----------------------------------------------------------------------------
std::vector<std::string> loader::getFiles()
{
  return this->Internals->FilesList;
}

//----------------------------------------------------------------------------
void loader::setCurrentFileIndex(int index)
{
  this->Internals->CurrentFileIndex = index;
}

//----------------------------------------------------------------------------
int loader::getCurrentFileIndex()
{
  return this->Internals->CurrentFileIndex;
}

//----------------------------------------------------------------------------
const options& loader::getOptions()
{
  return this->Internals->Options;
}

//----------------------------------------------------------------------------
void loader::setInteractor(interactor* interactor)
{
  this->Internals->Interactor = interactor;
  if (this->Internals->Interactor)
  {
    this->Internals->Interactor->SetLoader(this);
  }
}

//----------------------------------------------------------------------------
void loader::setWindow(window* window)
{
  this->Internals->Window = window;
  if (this->Internals->Window)
  {
    this->Internals->Window->SetOptions(&this->Internals->Options);
  }
}

//----------------------------------------------------------------------------
window* loader::getWindow()
{
  return this->Internals->Window;
}

//----------------------------------------------------------------------------
bool loader::loadFile(loader::LoadFileEnum load)
{
  // Reset loadedFile
  this->Internals->LoadedFile = false;

  F3DLog::SetQuiet(this->Internals->Options.get<bool>("quiet"));

  if (!this->Internals->Window)
  {
    F3DLog::Print(F3DLog::Severity::Error, "No window provided, aborting\n");
    return this->Internals->LoadedFile;
  }

  // Prevent the animation manager from playing
  this->Internals->AnimationManager.Finalize();

  // Recover information about the file to load
  std::string filePath, fileInfo;
  this->getFileInfo(load, this->Internals->CurrentFileIndex, filePath, fileInfo);
  bool verbose = this->Internals->Options.get<bool>("verbose");
  if (verbose)
  {
    if (filePath.empty())
    {
      F3DLog::Print(F3DLog::Severity::Info, "No file to load provided\n");
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Info, "Loading: ", filePath, "\n");
    }
  }

  // No file provided, show a drop zone instead
  if (filePath.empty())
  {
    fileInfo += "No file to load provided, please drop one into this window";
    this->Internals->Window->Initialize(false, fileInfo);
    this->Internals->Window->update();
    return this->Internals->LoadedFile;
  }

  // Recover the importer
  this->Internals->Importer = ::GetImporter(this->Internals->Options, filePath);
  vtkF3DGenericImporter* genericImporter =
    vtkF3DGenericImporter::SafeDownCast(this->Internals->Importer);
  if (!this->Internals->Importer)
  {
    F3DLog::Print(
      F3DLog::Severity::Warning, filePath, " is not a file of a supported file format\n");
    fileInfo += " [UNSUPPORTED]";
    this->Internals->Window->Initialize(false, fileInfo);
    this->Internals->Window->update();
    return this->Internals->LoadedFile;
  }

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  ::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;

  this->Internals->Window->Initialize(genericImporter != nullptr, fileInfo);

  // Initialize importer for rendering
  this->Internals->Importer->SetRenderWindow(this->Internals->Window->GetRenderWindow());
#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 0, 20210228)
  this->Internals->Importer->SetCamera(this->Internals->Options.get<int>("camera-index"));
#endif

  // Manage progress bar
  if (this->Internals->Options.get<bool>("progress") && this->Internals->Interactor)
  {
    ::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->Importer, this->Internals->Interactor);
  }

  // Read the file
  this->Internals->Importer->Update();
  if (verbose)
  {
    ::DisplayImporterDescription(this->Internals->Importer);
  }

  // Remove anything progress related if any
  this->Internals->Importer->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  // Initialize the animation manager using temporal
  // information from the importer
  // TODO improve this API
  this->Internals->AnimationManager.Initialize(
    this->Internals->Options, this->Internals->Importer, this->Internals->Window);

  // Recover generic importer specific actors and mappers to set on the renderer with coloring
  vtkF3DRendererWithColoring* renWithColor =
    vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Window->GetRenderer());
  if (renWithColor && genericImporter)
  {
    renWithColor->SetScalarBarActor(genericImporter->GetScalarBarActor());
    renWithColor->SetGeometryActor(genericImporter->GetGeometryActor());
    renWithColor->SetPointSpritesActor(genericImporter->GetPointSpritesActor());
    renWithColor->SetVolumeProp(genericImporter->GetVolumeProp());
    renWithColor->SetPolyDataMapper(genericImporter->GetPolyDataMapper());
    renWithColor->SetPointGaussianMapper(genericImporter->GetPointGaussianMapper());
    renWithColor->SetVolumeMapper(genericImporter->GetVolumeMapper());
    renWithColor->SetColoring(genericImporter->GetPointDataForColoring(),
      genericImporter->GetCellDataForColoring(), this->Internals->Options.get<bool>("cells"),
      genericImporter->GetArrayIndexForColoring(), this->Internals->Options.get<int>("component"));
  }

  // Initialize renderer using data read by the importer
  this->Internals->Window->update();

  this->Internals->LoadedFile = true;
  return this->Internals->LoadedFile;
}

//----------------------------------------------------------------------------
bool loader::start()
{
  // TODO rework this method completelly

  bool retVal = true;

  // Manage recording options
  vtkNew<vtkF3DInteractorEventRecorder> recorder;
  std::string interactionTestRecordFile =
    this->Internals->Options.get<std::string>("interaction-test-record");
  std::string interactionTestPlayFile =
    this->Internals->Options.get<std::string>("interaction-test-play");
  bool record = !interactionTestRecordFile.empty();
  bool play = !interactionTestPlayFile.empty();
  if (record || play)
  {
    if (!this->Internals->Interactor)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Cannot use interaction test record and play without an interactor");
      retVal = false;
    }
    else
    {
      this->Internals->Interactor->SetInteractorOn(recorder);
      if (record)
      {
        if (play)
        {
          F3DLog::Print(F3DLog::Severity::Warning,
            "Interaction test record and play files have been provided, play file ignored");
        }
        interactionTestRecordFile =
          vtksys::SystemTools::CollapseFullPath(interactionTestRecordFile);
        recorder->SetFileName(interactionTestRecordFile.c_str());
        recorder->On();
        recorder->Record();
      }
      else
      {
        if (!vtksys::SystemTools::FileExists(interactionTestPlayFile))
        {
          F3DLog::Print(F3DLog::Severity::Error, "Interaction record file to play does not exist ",
            interactionTestPlayFile);
          retVal = false;
        }
        else
        {
          interactionTestPlayFile = vtksys::SystemTools::CollapseFullPath(interactionTestPlayFile);
          recorder->SetFileName(interactionTestPlayFile.c_str());
          recorder->Play();
          recorder->Off();
        }
      }
    }
  }

  bool noBackground = this->Internals->Options.get<bool>("no-background");
  std::string reference = this->Internals->Options.get<std::string>("reference");
  std::string output = this->Internals->Options.get<std::string>("output");

  vtkRenderWindow* renWin = this->Internals->Window->GetRenderWindow();

  // Recorder can stop the interactor, make sure it is still running
  if (this->Internals->Interactor && this->Internals->Interactor->GetDone())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Interactor has been stopped, no rendering performed");
    retVal = false;
  }
  else if (!reference.empty())
  {
    if (!this->Internals->LoadedFile)
    {
      F3DLog::Print(F3DLog::Severity::Error, "No file loaded, no rendering performed");
      retVal = false;
    }
    else
    {
      retVal = F3DOffscreenRender::RenderTesting(renWin, reference,
        this->Internals->Options.get<double>("ref-threshold"), noBackground, output);
    }
  }
  else if (!output.empty())
  {
    if (!this->Internals->LoadedFile)
    {
      F3DLog::Print(F3DLog::Severity::Error, "No file loaded, no rendering performed");
      retVal = false;
    }
    else
    {
      retVal = F3DOffscreenRender::RenderOffScreen(renWin, output, noBackground);
    }
  }
  else if (this->Internals->Interactor)
  {
    renWin->Render();
    this->Internals->Interactor->Start();
  }
  else
  {
    F3DLog::Print(F3DLog::Severity::Warning, "No rendering performed");
    retVal = false;
  }

  // The axis widget should be disabled before destruction
  this->Internals->Window->GetRenderer()->ShowAxis(false);
  this->Internals->AnimationManager.Finalize();

  if (record)
  {
    recorder->Off();
  }
  return retVal;
}

//----------------------------------------------------------------------------
const F3DAnimationManager* loader::GetAnimationManager()
{
  return &this->Internals->AnimationManager;
}
}
