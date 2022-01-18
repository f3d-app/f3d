#include "F3DLoader.h"

#include "F3DAnimationManager.h"
#include "F3DLog.h"
#include "F3DOffscreenRender.h"
#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"
#include "f3d_options.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DInteractorStyle.h"
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
#include <vtkRenderWindowInteractor.h>
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
  ProgressDataStruct* data, vtkImporter* importer, vtkRenderWindowInteractor* interactor)
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
        widget->GetInteractor()->GetRenderWindow()->Render();
      }
    });
  importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);

  data->widget->SetInteractor(interactor);

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

void DisplayCameraInformation(vtkCamera* cam)
{
  double* position = cam->GetPosition();
  F3DLog::Print(F3DLog::Severity::Info, "Camera position is: ", position[0], ", ", position[1],
    ", ", position[2], ".");
  double* focalPoint = cam->GetFocalPoint();
  F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is: ", focalPoint[0], ", ",
    focalPoint[1], ", ", focalPoint[2], ".");
  double* viewUp = cam->GetViewUp();
  F3DLog::Print(F3DLog::Severity::Info, "Camera view up is: ", viewUp[0], ", ", viewUp[1], ", ",
    viewUp[2], ".");
  F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is: ", cam->GetViewAngle(), ".\n");
}
}

class F3DLoader::F3DInternals
{
public:
  std::vector<std::string> FilesList;
  int CurrentFileIndex = 0;
  bool LoadedFile = false;
  f3d::options Options;
  F3DAnimationManager AnimationManager;
  vtkSmartPointer<vtkF3DRenderer> Renderer;
  vtkSmartPointer<vtkImporter> Importer;
  vtkSmartPointer<vtkRenderWindow> RenWin;
  vtkSmartPointer<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
  F3DReaderInstantiator ReaderInstantiator;
};

//----------------------------------------------------------------------------
F3DLoader::F3DLoader()
  : Internals(new F3DLoader::F3DInternals)
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
F3DLoader::~F3DLoader() = default;

//----------------------------------------------------------------------------
vtkF3DInteractorStyle* F3DLoader::GetInteractorStyle()
{
  return this->Internals->Style;
}

//----------------------------------------------------------------------------
void F3DLoader::InitializeRendering(
  std::string windowName, bool offscreen, const void* icon, size_t iconSize)
{
  this->Internals->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
  this->Internals->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
  this->Internals->RenWin->SetWindowName(windowName.c_str());

  // Offscreen rendering must be set before initializing interactor
  this->Internals->RenWin->SetOffScreenRendering(offscreen);

  this->Internals->Style = vtkSmartPointer<vtkF3DInteractorStyle>::New();
  this->Internals->Style->SetAnimationManager(this->Internals->AnimationManager);

  this->Internals->Interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  this->Internals->Interactor->SetRenderWindow(this->Internals->RenWin);
  this->Internals->Interactor->SetInteractorStyle(this->Internals->Style);
  this->Internals->Interactor->Initialize();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200615)
  // set icon
  vtkNew<vtkPNGReader> iconReader;
  iconReader->SetMemoryBuffer(icon);
  iconReader->SetMemoryBufferLength(iconSize);
  iconReader->Update();
  this->Internals->RenWin->SetIcon(iconReader->GetOutput());
#else
  (void)icon;
  (void)iconSize;
#endif
}

//----------------------------------------------------------------------------
void F3DLoader::ToggleAnimation()
{
  this->Internals->AnimationManager.ToggleAnimation();
}

//----------------------------------------------------------------------------
void F3DLoader::AddFiles(const std::vector<std::string>& files)
{
  for (auto& file : files)
  {
    this->AddFile(file);
  }
}

//----------------------------------------------------------------------------
void F3DLoader::AddFile(const std::string& path, bool recursive)
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
        this->AddFile(vtksys::SystemTools::JoinPath({ "", fullPath, currentFile }), false);
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
void F3DLoader::GetNextFile(
  LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const
{
  int size = static_cast<int>(this->Internals->FilesList.size());
  if (size > 0)
  {
    int addToIndex = 0;
    switch (load)
    {
      case F3DLoader::LoadFileEnum::LOAD_FIRST:
        nextFileIndex = 0;
        break;
      case F3DLoader::LoadFileEnum::LOAD_LAST:
        nextFileIndex = size - 1;
        break;
      case F3DLoader::LoadFileEnum::LOAD_PREVIOUS:
        addToIndex = -1;
        break;
      case F3DLoader::LoadFileEnum::LOAD_NEXT:
        addToIndex = 1;
        break;
      case F3DLoader::LoadFileEnum::LOAD_CURRENT:
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
bool F3DLoader::LoadFile(F3DLoader::LoadFileEnum load, const f3d::options& options)
{
  // Reset loadedFile
  this->Internals->LoadedFile = false;

  // TODO should Copy provided options
  this->Internals->Options = options;

  // Respect quiet option
  F3DLog::SetQuiet(this->Internals->Options.get<bool>("quiet"));

  // XXX May need to go somewhere else
  // Will only be used when interacting with a animated file
  if (this->Internals->Style)
  {
    this->Internals->Style->SetOptions(&this->Internals->Options);
  }

  // Prevent the animation manager from playing
  this->Internals->AnimationManager.Finalize();

  // Clear renderer if already present
  if (this->Internals->Renderer)
  {
    this->Internals->Renderer->ShowAxis(false);
    this->Internals->RenWin->RemoveRenderer(this->Internals->Renderer);
  }

  if (this->Internals->RenWin)
  {
    this->Internals->RenWin->SetSize(
      this->Internals->Options.get<std::vector<int> >("resolution").data());
    this->Internals->RenWin->SetFullScreen(this->Internals->Options.get<bool>("fullscreen"));
  }

  // Recover information about the file to load
  std::string filePath, fileInfo;
  this->GetNextFile(load, this->Internals->CurrentFileIndex, filePath, fileInfo);
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
    if (this->Internals->RenWin)
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
      this->Internals->RenWin->AddRenderer(this->Internals->Renderer);

      fileInfo += "No file to load provided, please drop one into this window";
      this->Internals->Renderer->Initialize(this->Internals->Options, fileInfo);
      this->Internals->Renderer->ShowOptions();
    }
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
    if (this->Internals->RenWin)
    {
      fileInfo += " [UNSUPPORTED]";
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
      this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
      this->Internals->Renderer->Initialize(this->Internals->Options, fileInfo);
      this->Internals->Renderer->ShowOptions();
    }
    return this->Internals->LoadedFile;
  }

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  ::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;
  if (this->Internals->RenWin)
  {
    // Create and initialize renderer from options
    if (genericImporter)
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRendererWithColoring>::New();
    }
    else
    {
      this->Internals->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
    }
    this->Internals->RenWin->AddRenderer(this->Internals->Renderer);
    this->Internals->Renderer->Initialize(this->Internals->Options, fileInfo);

    // Initialize importer for rendering
    this->Internals->Importer->SetRenderWindow(this->Internals->Renderer->GetRenderWindow());
#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 0, 20210228)
    this->Internals->Importer->SetCamera(this->Internals->Options.get<int>("camera-index"));
#endif

    // Manage progress bar
    if (this->Internals->Options.get<bool>("progress"))
    {
      ::CreateProgressRepresentationAndCallback(
        &callbackData, this->Internals->Importer, this->Internals->Interactor);
    }
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
  if (this->Internals->RenWin)
  {
    this->Internals->AnimationManager.Initialize(this->Internals->Options,
      this->Internals->Importer, this->Internals->RenWin, this->Internals->Renderer);
  }

  // Initialize renderer using data read by the importer
  this->InitializeRenderer();

  this->Internals->LoadedFile = true;
  return this->Internals->LoadedFile;
}

//----------------------------------------------------------------------------
void F3DLoader::InitializeRenderer()
{
  if (this->Internals->Renderer)
  {
    // Recover generic importer specific actors and mappers to set on the renderer with coloring
    vtkF3DGenericImporter* genericImporter =
      vtkF3DGenericImporter::SafeDownCast(this->Internals->Importer);
    if (genericImporter)
    {
      // no sanity test needed
      vtkF3DRendererWithColoring* renWithColor =
        vtkF3DRendererWithColoring::SafeDownCast(this->Internals->Renderer);

      renWithColor->SetScalarBarActor(genericImporter->GetScalarBarActor());
      renWithColor->SetGeometryActor(genericImporter->GetGeometryActor());
      renWithColor->SetPointSpritesActor(genericImporter->GetPointSpritesActor());
      renWithColor->SetVolumeProp(genericImporter->GetVolumeProp());
      renWithColor->SetPolyDataMapper(genericImporter->GetPolyDataMapper());
      renWithColor->SetPointGaussianMapper(genericImporter->GetPointGaussianMapper());
      renWithColor->SetVolumeMapper(genericImporter->GetVolumeMapper());
      renWithColor->SetColoring(genericImporter->GetPointDataForColoring(),
        genericImporter->GetCellDataForColoring(), this->Internals->Options.get<bool>("cells"),
        genericImporter->GetArrayIndexForColoring(),
        this->Internals->Options.get<int>("component"));
    }

    // Actors are loaded, use the bounds to reset camera and set-up SSAO
    this->Internals->Renderer->SetupRenderPasses();
    this->Internals->Renderer->UpdateInternalActors();
    this->Internals->Renderer->ShowOptions();

    // Set the initial camera once all options
    // have been shown as they may have an effect on it
    if (this->Internals->Options.get<int>("camera-index") < 0)
    {
      // set a default camera from bounds using VTK method
      this->Internals->Renderer->vtkRenderer::ResetCamera();

      // use options to overwrite camera parameters
      vtkCamera* cam = this->Internals->Renderer->GetActiveCamera();

      std::vector<double> cameraPosition =
        this->Internals->Options.get<std::vector<double> >("camera-position");
      if (cameraPosition.size() == 3)
      {
        cam->SetPosition(cameraPosition.data());
      }

      std::vector<double> cameraFocalPoint =
        this->Internals->Options.get<std::vector<double> >("camera-focal-point");
      if (cameraFocalPoint.size() == 3)
      {
        cam->SetFocalPoint(cameraFocalPoint.data());
      }

      std::vector<double> cameraViewUp =
        this->Internals->Options.get<std::vector<double> >("camera-view-up");
      if (cameraViewUp.size() == 3)
      {
        cam->SetViewUp(cameraViewUp.data());
      }

      double cameraViewAngle = this->Internals->Options.get<double>("camera-view-angle");
      if (cameraViewAngle != 0)
      {
        cam->SetViewAngle(cameraViewAngle);
      }

      cam->Azimuth(this->Internals->Options.get<double>("camera-azimuth-angle"));
      cam->Elevation(this->Internals->Options.get<double>("camera-elevation-angle"));
      cam->OrthogonalizeViewUp();

      if (this->Internals->Options.get<bool>("verbose"))
      {
        ::DisplayCameraInformation(cam);
      }
    }

    this->Internals->Renderer->InitializeCamera();
  }
}

//----------------------------------------------------------------------------
bool F3DLoader::Start()
{
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
    recorder->SetInteractor(this->Internals->Interactor);
    if (record)
    {
      if (play)
      {
        F3DLog::Print(F3DLog::Severity::Warning,
          "Interaction test record and play files have been provided, play file ignored");
      }
      interactionTestRecordFile = vtksys::SystemTools::CollapseFullPath(interactionTestRecordFile);
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

  bool noBackground = this->Internals->Options.get<bool>("no-background");
  std::string reference = this->Internals->Options.get<std::string>("reference");
  std::string output = this->Internals->Options.get<std::string>("output");

  // Recorder can stop the interactor, make sure it is still running
  if (this->Internals->Interactor->GetDone())
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
      retVal = F3DOffscreenRender::RenderTesting(this->Internals->RenWin, reference,
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
      retVal = F3DOffscreenRender::RenderOffScreen(this->Internals->RenWin, output, noBackground);
    }
  }
  else
  {
    this->Internals->RenWin->Render();
    this->Internals->Interactor->Start();
  }

  // The axis widget should be disabled before destruction
  this->Internals->Renderer->ShowAxis(false);
  this->Internals->AnimationManager.Finalize();

  if (record)
  {
    recorder->Off();
  }
  return retVal;
}
