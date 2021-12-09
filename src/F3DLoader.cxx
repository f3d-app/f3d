#include "F3DLoader.h"

#include "F3DLog.h"
#include "F3DNSDelegate.h"
#include "F3DOffscreenRender.h"
#include "F3DOptions.h"
#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DInteractorStyle2D.h"
#include "vtkF3DInteractorStyle3D.h"
#include "vtkF3DInteractionHandler.h"
#include "vtkF3DObjectFactory.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkStringArray.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <algorithm>

#include "F3DIcon.h"

typedef struct ProgressDataStruct
{
  vtkTimerLog* timer;
  vtkProgressBarWidget* widget;
} ProgressDataStruct;

//----------------------------------------------------------------------------
F3DLoader::F3DLoader()
{
  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  this->ReaderInstantiator = new F3DReaderInstantiator();
}

//----------------------------------------------------------------------------
F3DLoader::~F3DLoader()
{
  delete this->ReaderInstantiator;
}

//----------------------------------------------------------------------------
void F3DLoader::OnNewFiles(vtkObject*, unsigned long, void* clientData, void* callData)
{
  F3DLoader* that = static_cast<F3DLoader*>(clientData);
  that->CurrentFileIndex = static_cast<int>(that->FilesList.size());
  vtkStringArray* files = static_cast<vtkStringArray*>(callData);
  for (int i = 0; i < files->GetNumberOfTuples(); i++)
  {
    that->AddFile(files->GetValue(i));
  }
  that->LoadFile();
}

//----------------------------------------------------------------------------
void F3DLoader::OnLoadFile(vtkObject*, unsigned long, void* clientData, void* callData)
{
  F3DLoader* that = static_cast<F3DLoader*>(clientData);
  int* load = static_cast<int*>(callData);
  that->LoadFile(*load);
}

//----------------------------------------------------------------------------
void F3DLoader::OnToggleAnimation(vtkObject*, unsigned long, void* clientData, void*)
{
  F3DAnimationManager* animMgr = static_cast<F3DAnimationManager*>(clientData);
  animMgr->ToggleAnimation();
}

//----------------------------------------------------------------------------
int F3DLoader::Start(int argc, char** argv, vtkImageData* image)
{
  std::vector<std::string> files;

  this->Parser.Initialize(argc, argv);
  this->CommandLineOptions = this->Parser.GetOptionsFromCommandLine(files);
  F3DLog::SetQuiet(this->CommandLineOptions.Quiet);
  this->Parser.InitializeDictionaryFromConfigFile(this->CommandLineOptions.UserConfigFile);

  vtkNew<vtkRenderWindowInteractor> interactor;
  if (!this->CommandLineOptions.NoRender)
  {
    this->RenWin = vtkSmartPointer<vtkRenderWindow>::New();
    this->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
    this->RenWin->SetWindowName(f3d::AppTitle.c_str());

    this->InteractionHandler = vtkSmartPointer<vtkF3DInteractionHandler>::New();

    // Setup the observers for the interactor style events
    vtkNew<vtkCallbackCommand> newFilesCallback;
    newFilesCallback->SetClientData(this);
    newFilesCallback->SetCallback(F3DLoader::OnNewFiles);
    this->InteractionHandler->AddObserver(F3DLoader::NewFilesEvent, newFilesCallback);

    vtkNew<vtkCallbackCommand> loadFileCallback;
    loadFileCallback->SetClientData(this);
    loadFileCallback->SetCallback(F3DLoader::OnLoadFile);
    this->InteractionHandler->AddObserver(F3DLoader::LoadFileEvent, loadFileCallback);

    vtkNew<vtkCallbackCommand> toggleAnimationCallback;
    toggleAnimationCallback->SetClientData(&this->AnimationManager);
    toggleAnimationCallback->SetCallback(F3DLoader::OnToggleAnimation);
    this->InteractionHandler->AddObserver(F3DLoader::ToggleAnimationEvent, toggleAnimationCallback);

    // Offscreen rendering must be set before initializing interactor
    if (!this->CommandLineOptions.Reference.empty() || !this->CommandLineOptions.Output.empty() ||
      image)
    {
      this->RenWin->OffScreenRenderingOn();
    }

    interactor->SetRenderWindow(this->RenWin);
    this->InteractionHandler->SetupInteractorStyles(interactor, &this->AnimationManager, &this->Options);
    interactor->Initialize();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200615)
    // set icon
    vtkNew<vtkImageData> icon;
    icon->SetDimensions(F3DIconDimX, F3DIconDimY, 1);
    icon->AllocateScalars(VTK_UNSIGNED_CHAR, F3DIconNbComp);

    unsigned char* p = static_cast<unsigned char*>(icon->GetScalarPointer(0, 0, 0));
    std::copy(F3DIconBuffer, F3DIconBuffer + F3DIconDimX * F3DIconDimY * F3DIconNbComp, p);

    this->RenWin->SetIcon(icon);
#endif

#if __APPLE__
    F3DNSDelegate::InitializeDelegate(this, this->RenWin);
#endif
  }

  this->AddFiles(files);
  bool loaded = this->LoadFile();
  int retVal = EXIT_SUCCESS;

  // Actual Options been parsed in LoadFile so use them
  if (!this->Options.NoRender)
  {
    // Manage recording options
    vtkNew<vtkF3DInteractorEventRecorder> recorder;
    bool record = !this->Options.InteractionTestRecordFile.empty();
    bool play = !this->Options.InteractionTestPlayFile.empty();
    if (record || play)
    {
      recorder->SetInteractor(interactor);
      if (record)
      {
        if (play)
        {
          F3DLog::Print(F3DLog::Severity::Warning,
            "Interaction test record and play files have been provided, play file ignored.");
        }
        recorder->SetFileName(this->Options.InteractionTestRecordFile.c_str());
        recorder->On();
        recorder->Record();
      }
      else
      {
        recorder->SetFileName(this->Options.InteractionTestPlayFile.c_str());
        recorder->Play();
        recorder->Off();
      }
    }

    // Recorder can stop the interactor, make sure it is still running
    if (interactor->GetDone())
    {
      F3DLog::Print(
        F3DLog::Severity::Warning, "Interactor has been stopped, no rendering performed");
      retVal = EXIT_FAILURE;
    }
    else if (!this->Options.Reference.empty())
    {
      if (!loaded)
      {
        F3DLog::Print(F3DLog::Severity::Warning, "No file loaded, no rendering performed");
        retVal = EXIT_FAILURE;
      }
      else
      {
        retVal = F3DOffscreenRender::RenderTesting(this->RenWin, this->Options.Reference,
                   this->Options.RefThreshold, this->Options.NoBackground, this->Options.Output)
          ? EXIT_SUCCESS
          : EXIT_FAILURE;
      }
    }
    else if (!this->Options.Output.empty())
    {
      if (!loaded)
      {
        F3DLog::Print(F3DLog::Severity::Warning, "No file loaded, no rendering performed");
        retVal = EXIT_FAILURE;
      }
      else
      {
        retVal = F3DOffscreenRender::RenderOffScreen(
                   this->RenWin, this->Options.Output, this->Options.NoBackground)
          ? EXIT_SUCCESS
          : EXIT_FAILURE;
      }
    }
    else if (image)
    {
      retVal = F3DOffscreenRender::RenderToImage(this->RenWin, image, this->Options.NoBackground)
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
    }
    else
    {
      this->RenWin->Render();
      interactor->Start();
    }

    // The axis widget should be disabled before destruction
    this->Renderer->ShowAxis(false);
    this->AnimationManager.Finalize();

    if (record)
    {
      recorder->Off();
    }
  }

  return retVal;
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
  if (path.empty() || !vtksys::SystemTools::FileExists(path))
  {
    F3DLog::Print(F3DLog::Severity::Error, "File ", path, " does not exist");
    return;
  }

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);

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
    auto it = std::find(this->FilesList.begin(), this->FilesList.end(), fullPath);

    if (it == this->FilesList.end())
    {
      this->FilesList.push_back(fullPath);
    }
  }
}

//----------------------------------------------------------------------------
// TODO this method is long and complex, this should be improved
bool F3DLoader::LoadFile(int load)
{
  // Prevent the animation manager from playing
  this->AnimationManager.Finalize();

  // Clear renderer if already present
  if (this->Renderer)
  {
    this->Renderer->ShowAxis(false);
    this->RenWin->RemoveRenderer(this->Renderer);
  }

  std::string filePath, fileInfo;
  int size = static_cast<int>(this->FilesList.size());
  if (size > 0)
  {
    // Compute the correct file index
    this->CurrentFileIndex = (this->CurrentFileIndex + load) % size;
    this->CurrentFileIndex =
      this->CurrentFileIndex < 0 ? this->CurrentFileIndex + size : this->CurrentFileIndex;

    if (this->CurrentFileIndex >= size)
    {
      F3DLog::Print(F3DLog::Severity::Error, "Cannot load file index ", this->CurrentFileIndex);
      return false;
    }
    filePath = this->FilesList[this->CurrentFileIndex];
    fileInfo = "(" + std::to_string(this->CurrentFileIndex + 1) + "/" + std::to_string(size) +
      ") " + vtksys::SystemTools::GetFilenameName(filePath);
  }

  if (this->CommandLineOptions.DryRun)
  {
    this->Options = this->CommandLineOptions;
  }
  else
  {
    this->Options = this->Parser.GetOptionsFromConfigFile(filePath);
  }

  F3DLog::SetQuiet(this->Options.Quiet);

  if (this->Options.Verbose || this->Options.NoRender)
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

  if (!this->Options.NoRender)
  {
    this->RenWin->SetSize(this->Options.WindowSize[0], this->Options.WindowSize[1]);
    this->RenWin->SetFullScreen(this->Options.FullScreen);
  }

  bool loaded = false;
  if (filePath.empty())
  {
    if (!this->Options.NoRender)
    {
      this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
      this->RenWin->AddRenderer(this->Renderer);

      fileInfo += "No file to load provided, please drop one into this window";
      this->Renderer->Initialize(this->Options, fileInfo);
      this->Renderer->ShowOptions();
    }
    return loaded;
  }

  this->Importer = this->GetImporter(this->Options, filePath);
  vtkF3DGenericImporter* genericImporter = vtkF3DGenericImporter::SafeDownCast(this->Importer);

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  ProgressDataStruct data;
  data.timer = timer.Get();
  data.widget = progressWidget.Get();

  if (!this->Importer)
  {
    F3DLog::Print(
      F3DLog::Severity::Warning, filePath, " is not a file of a supported file format\n");
    if (!this->Options.NoRender)
    {
      fileInfo += " [UNSUPPORTED]";
      this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
      this->RenWin->AddRenderer(this->Renderer);
      this->Renderer->Initialize(this->Options, fileInfo);
      this->Renderer->ShowOptions();
    }
    return loaded;
  }
  else
  {
    loaded = true;
  }

  if (!this->Options.NoRender)
  {
    // Create and initialize renderer
    if (genericImporter)
    {
      this->Renderer = vtkSmartPointer<vtkF3DRendererWithColoring>::New();
    }
    else
    {
      this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
    }
    this->RenWin->AddRenderer(this->Renderer);
    this->Renderer->Initialize(this->Options, fileInfo);

    this->Importer->SetRenderWindow(this->Renderer->GetRenderWindow());

#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 0, 20210228)
    this->Importer->SetCamera(this->Options.CameraIndex);
#endif

    if (this->Options.Progress)
    {
      vtkNew<vtkCallbackCommand> progressCallback;
      progressCallback->SetClientData(&data);
      progressCallback->SetCallback(
        [](vtkObject*, unsigned long, void* clientData, void* callData)
        {
          auto data = static_cast<ProgressDataStruct*>(clientData);
          data->timer->StopTimer();
          vtkProgressBarWidget* widget = data->widget;
          // Only show and render the progress bar if loading takes more than 0.15 seconds
          if (data->timer->GetElapsedTime() > 0.15)
          {
            widget->On();
            vtkProgressBarRepresentation* rep =
              vtkProgressBarRepresentation::SafeDownCast(widget->GetRepresentation());
            rep->SetProgressRate(*static_cast<double*>(callData));
            widget->GetInteractor()->GetRenderWindow()->Render();
          }
        });
      this->Importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);

      progressWidget->SetInteractor(this->Renderer->GetRenderWindow()->GetInteractor());

      vtkProgressBarRepresentation* progressRep =
        vtkProgressBarRepresentation::SafeDownCast(progressWidget->GetRepresentation());
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
    }
  }

  timer->StartTimer();
  this->Importer->Update();

  // we need to remove progress observer in order to hide the progress bar during animation
  this->Importer->RemoveObservers(vtkCommand::ProgressEvent);

  if (!this->Options.NoRender)
  {
    this->AnimationManager.Initialize(this->Options, this->Importer, this->RenWin, this->Renderer);
  }

  // Display description
  if (this->Options.Verbose || this->Options.NoRender)
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210228)
    vtkIdType availCameras = this->Importer->GetNumberOfCameras();
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
      F3DLog::Print(F3DLog::Severity::Info, i, ": ", this->Importer->GetCameraName(i));
    }
    F3DLog::Print(F3DLog::Severity::Info, "\n");
#endif
    F3DLog::Print(F3DLog::Severity::Info, this->Importer->GetOutputsDescription());
  }

  if (!this->Options.NoRender)
  {
    progressWidget->Off();

    // Recover generic importer specific actors and mappers to set on the renderer with coloring
    if (genericImporter)
    {
      // no sanity test needed
      vtkF3DRendererWithColoring* renWithColor =
        vtkF3DRendererWithColoring::SafeDownCast(this->Renderer);

      renWithColor->SetScalarBarActor(genericImporter->GetScalarBarActor());
      renWithColor->SetGeometryActor(genericImporter->GetGeometryActor());
      renWithColor->SetPointSpritesActor(genericImporter->GetPointSpritesActor());
      renWithColor->SetVolumeProp(genericImporter->GetVolumeProp());
      renWithColor->SetPolyDataMapper(genericImporter->GetPolyDataMapper());
      renWithColor->SetPointGaussianMapper(genericImporter->GetPointGaussianMapper());
      renWithColor->SetVolumeMapper(genericImporter->GetVolumeMapper());
      renWithColor->SetColoring(genericImporter->GetPointDataForColoring(),
        genericImporter->GetCellDataForColoring(), this->Options.Cells,
        genericImporter->GetArrayIndexForColoring(), this->Options.Component);
    }

    // Actors are loaded, use the bounds to reset camera and set-up SSAO
    this->Renderer->SetupRenderPasses();
    this->Renderer->UpdateInternalActors();
    this->Renderer->ShowOptions();

    this->InteractionHandler->SetDefaultStyle(this->Renderer);

    // Set the initial camera once all options
    // have been shown as they may have an effect on it
    if (this->Options.CameraIndex < 0)
    {
      // set a default camera from bounds using VTK method
      this->Renderer->vtkRenderer::ResetCamera();

      // use options to overwrite camera parameters
      vtkCamera* cam = this->Renderer->GetActiveCamera();
      if (this->Options.CameraPosition.size() == 3)
      {
        cam->SetPosition(this->Options.CameraPosition.data());
      }
      if (this->Options.CameraFocalPoint.size() == 3)
      {
        cam->SetFocalPoint(this->Options.CameraFocalPoint.data());
      }
      if (this->Options.CameraViewUp.size() == 3)
      {
        cam->SetViewUp(this->Options.CameraViewUp.data());
      }
      if (this->Options.CameraViewAngle != 0)
      {
        cam->SetViewAngle(this->Options.CameraViewAngle);
      }
      cam->Azimuth(this->Options.CameraAzimuthAngle);
      cam->Elevation(this->Options.CameraElevationAngle);
      cam->OrthogonalizeViewUp();
      if (this->Options.Verbose)
      {
        double* position = cam->GetPosition();
        F3DLog::Print(F3DLog::Severity::Info, "Camera position is: ", position[0], ", ",
          position[1], ", ", position[2], ".");
        double* focalPoint = cam->GetFocalPoint();
        F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is: ", focalPoint[0], ", ",
          focalPoint[1], ", ", focalPoint[2], ".");
        double* viewUp = cam->GetViewUp();
        F3DLog::Print(F3DLog::Severity::Info, "Camera view up is: ", viewUp[0], ", ", viewUp[1],
          ", ", viewUp[2], ".");
        F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is: ", cam->GetViewAngle(), ".\n");
      }
    }

    this->Renderer->InitializeCamera();
  }
  return loaded;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImporter> F3DLoader::GetImporter(
  const F3DOptions& options, const std::string& fileName)
{
  if (!options.GeometryOnly)
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
  importer->SetOptions(options);
  if (!importer->CanReadFile())
  {
    return nullptr;
  }
  return importer;
}
