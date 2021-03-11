#include "F3DLoader.h"

#include "F3DLog.h"
#include "F3DNSDelegate.h"
#include "F3DOffscreenRender.h"
#include "F3DOptions.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtk3DSImporter.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkDoubleArray.h>
#include <vtkGLTFImporter.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkStringArray.h>
#include <vtkTimerLog.h>
#include <vtkVRMLImporter.h>
#include <vtkVersion.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <algorithm>

#include "F3DIcon.h"

typedef struct ProgressDataStruct {
    vtkTimerLog* timer;
    vtkProgressBarWidget* widget;
} ProgressDataStruct;

//----------------------------------------------------------------------------
F3DLoader::F3DLoader() = default;

//----------------------------------------------------------------------------
F3DLoader::~F3DLoader() = default;

//----------------------------------------------------------------------------
int F3DLoader::Start(int argc, char** argv)
{
  std::vector<std::string> files;

  this->Parser.Initialize(argc, argv);
  F3DOptions options = this->Parser.GetOptionsFromCommandLine(files);

  this->RenWin = vtkSmartPointer<vtkRenderWindow>::New();

  vtkNew<vtkRenderWindowInteractor> interactor;
  if (!options.NoRender)
  {
    this->RenWin->SetSize(options.WindowSize[0], options.WindowSize[1]);
    this->RenWin->SetMultiSamples(0); // Disable hardware antialiasing
    this->RenWin->SetFullScreen(options.FullScreen);
    this->RenWin->SetWindowName(f3d::AppTitle.c_str());

    vtkNew<vtkF3DInteractorStyle> style;
    style->SetAnimationManager(this->AnimationManager);
    style->SetOptions(this->Options);

    // Setup the observers for the interactor style events
    vtkNew<vtkCallbackCommand> newFilesCallback;
    newFilesCallback->SetClientData(this);
    newFilesCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void* callData) {
      F3DLoader* loader = static_cast<F3DLoader*>(clientData);
      loader->CurrentFileIndex = static_cast<int>(loader->FilesList.size());
      vtkStringArray* files = static_cast<vtkStringArray*>(callData);
      for (int i = 0; i < files->GetNumberOfTuples(); i++)
      {
        loader->AddFile(files->GetValue(i));
      }
      loader->LoadFile();
    });
    style->AddObserver(F3DLoader::NewFilesEvent, newFilesCallback);

    vtkNew<vtkCallbackCommand> loadFileCallback;
    loadFileCallback->SetClientData(this);
    loadFileCallback->SetCallback([](vtkObject*, unsigned long, void* clientData, void* callData) {
      F3DLoader* loader = static_cast<F3DLoader*>(clientData);
      int* load = static_cast<int*>(callData);
      loader->LoadFile(*load);
    });
    style->AddObserver(F3DLoader::LoadFileEvent, loadFileCallback);

    vtkNew<vtkCallbackCommand> toggleAnimationCallback;
    toggleAnimationCallback->SetClientData(&this->AnimationManager);
    toggleAnimationCallback->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void*) {
        F3DAnimationManager* animMgr = static_cast<F3DAnimationManager*>(clientData);
        animMgr->ToggleAnimation();
      });
    style->AddObserver(F3DLoader::ToggleAnimationEvent, toggleAnimationCallback);

    interactor->SetRenderWindow(this->RenWin);
    interactor->SetInteractorStyle(style);
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
  this->LoadFile();

  int retVal = EXIT_SUCCESS;
  if (!options.NoRender)
  {
    if (!options.Output.empty())
    {
      retVal =
        F3DOffscreenRender::RenderOffScreen(this->RenWin, options.Output, options.NoBackground);
    }
    else if (!options.Reference.empty())
    {
      retVal =
        F3DOffscreenRender::RenderTesting(this->RenWin, options.Reference, options.RefThreshold);
    }
    else
    {
      this->RenWin->Render();
      interactor->Start();
    }

    // The axis widget should be disabled before destruction
    this->Renderer->ShowAxis(false);
    this->AnimationManager.Finalize();
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

      for (unsigned long i = 0; i < dir.GetNumberOfFiles(); i++)
      {
        std::string currentFile = dir.GetFile(i);
        if (currentFile != "." && currentFile != "..")
        {
          this->AddFile(vtksys::SystemTools::JoinPath({ "", fullPath, currentFile }), false);
        }
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
void F3DLoader::LoadFile(int load)
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
      return;
    }
    filePath = this->FilesList[this->CurrentFileIndex];
    fileInfo = "(" + std::to_string(this->CurrentFileIndex + 1) + "/" + std::to_string(size) +
      ") " + vtksys::SystemTools::GetFilenameName(filePath);
  }

  this->Options = this->Parser.GetOptionsFromCommandLine();
  if (!this->Options.DryRun)
  {
    this->Options = this->Parser.GetOptionsFromConfigFile(filePath);
  }

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

  if (filePath.empty())
  {
    this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
    this->RenWin->AddRenderer(this->Renderer);

    fileInfo += "No file to load provided, please drop one into this window";
    this->Renderer->Initialize(this->Options, fileInfo);
    this->Renderer->ShowOptions();
    return;
  }

  this->Importer = this->GetImporter(this->Options, filePath);
  vtkF3DGenericImporter* genericImporter = vtkF3DGenericImporter::SafeDownCast(this->Importer);

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  ProgressDataStruct data;
  data.timer = timer.Get();
  data.widget = progressWidget.Get();
  if (!this->Options.NoRender)
  {
    if (!this->Importer)
    {
      F3DLog::Print(
        F3DLog::Severity::Warning, filePath, " is not a file of a supported file format\n");
      fileInfo += " [UNSUPPORTED]";
      this->Renderer = vtkSmartPointer<vtkF3DRenderer>::New();
      this->RenWin->AddRenderer(this->Renderer);
      this->Renderer->Initialize(this->Options, fileInfo);
      this->Renderer->ShowOptions();
      return;
    }

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
        [](vtkObject*, unsigned long, void* clientData, void* callData) {
          auto data = static_cast<ProgressDataStruct*>(clientData);
          data->timer->StopTimer();
          vtkProgressBarWidget* widget = data->widget;
          // Only show and render the progress bar if loading takes more than 0.15 seconds
          if(data->timer->GetElapsedTime() > 0.15)
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

  this->AnimationManager.Initialize(this->Options, this->Importer, this->RenWin, this->Renderer);

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
      vtkF3DRendererWithColoring* renWithColor = vtkF3DRendererWithColoring::SafeDownCast(this->Renderer);

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
      cam->OrthogonalizeViewUp();
      if (this->Options.Verbose)
      {
        double* position = cam->GetPosition();
        F3DLog::Print(F3DLog::Severity::Info, "Camera position is : ", position[0], ", ", position[1],
          ", ", position[2], ".");
        double* focalPoint = cam->GetFocalPoint();
        F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is : ", focalPoint[0], ", ",
          focalPoint[1], ", ", focalPoint[2], ".");
        double* viewUp = cam->GetViewUp();
        F3DLog::Print(F3DLog::Severity::Info, "Camera view up is : ", viewUp[0], ", ", viewUp[1], ", ",
          viewUp[2], ".");
        F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is : ", cam->GetViewAngle(), ".\n");
      }
    }

    this->Renderer->InitializeCamera();
  }
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImporter> F3DLoader::GetImporter(
  const F3DOptions& options, const std::string& file)
{
  if (!vtksys::SystemTools::FileExists(file))
  {
    F3DLog::Print(F3DLog::Severity::Error, "Specified input file '", file, "' does not exist!");
    return nullptr;
  }

  if (!options.GeometryOnly)
  {
    std::string ext = vtksys::SystemTools::GetFilenameLastExtension(file);
    ext = vtksys::SystemTools::LowerCase(ext);

    if (ext == ".3ds")
    {
      vtkNew<vtk3DSImporter> importer;
      importer->SetFileName(file.c_str());
      importer->ComputeNormalsOn();
      return importer;
    }

    if (ext == ".obj")
    {
      vtkNew<vtkOBJImporter> importer;
      importer->SetFileName(file.c_str());

      std::string path = vtksys::SystemTools::GetFilenamePath(file);
      importer->SetTexturePath(path.c_str());

#if VTK_VERSION_NUMBER <= VTK_VERSION_CHECK(9, 0, 20201129)
      // This logic is partially implemented in the OBJ importer itself
      // This has been backported in VTK 9.1
      std::string mtlFile = file + ".mtl";
      if (vtksys::SystemTools::FileExists(mtlFile))
      {
        importer->SetFileNameMTL(mtlFile.c_str());
      }
      else
      {
        mtlFile = path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(file) + ".mtl";
        if (vtksys::SystemTools::FileExists(mtlFile))
        {
          importer->SetFileNameMTL(mtlFile.c_str());
        }
      }
#endif
      return importer;
    }

    if (ext == ".wrl")
    {
      vtkNew<vtkVRMLImporter> importer;
      importer->SetFileName(file.c_str());
      return importer;
    }

    if (ext == ".gltf" || ext == ".glb")
    {
      vtkNew<vtkGLTFImporter> importer;
      importer->SetFileName(file.c_str());
      return importer;
    }
  }

  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetFileName(file.c_str());
  importer->SetOptions(options);

  if (!importer->CanReadFile())
  {
    return nullptr;
  }

  return importer;
}
