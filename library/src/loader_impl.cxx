#include "loader_impl.h"

#include "animationManager.h"
#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "factory.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DMemoryMesh.h"

#include <vtkCallbackCommand.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <algorithm>
#include <numeric>
#include <vector>

namespace f3d::detail
{
class loader_impl::internals
{
public:
  internals(const options& options, window_impl& window)
    : Options(options)
    , Window(window)
  {
  }

  struct ProgressDataStruct
  {
    vtkTimerLog* timer;
    vtkProgressBarWidget* widget;
  };

  static void CreateProgressRepresentationAndCallback(
    ProgressDataStruct* data, vtkImporter* importer, interactor_impl* interactor)
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
        if (progressData->timer->GetElapsedTime() > 0.15 ||
          vtksys::SystemTools::HasEnv("CTEST_F3D_PROGRESS_BAR"))
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
    progressRep->DrawFrameOff();
    progressRep->SetPadding(0.0, 0.0);
    data->timer->StartTimer();
  }

  static void DisplayImporterDescription(vtkImporter* importer)
  {
    vtkIdType availCameras = importer->GetNumberOfCameras();
    if (availCameras <= 0)
    {
      log::debug("No camera available in this file");
    }
    else
    {
      log::debug("Camera(s) available in this file are:");
    }
    for (int i = 0; i < availCameras; i++)
    {
      log::debug(i, ": ", importer->GetCameraName(i));
    }
    log::debug("");
    log::debug(importer->GetOutputsDescription(), "\n");
  }

  void Reset()
  {
    // Reset the generic importer
    this->GenericImporter->RemoveInternalReaders();

    // Remove the importer from the renderer
    this->Window.SetImporterForColoring(nullptr);

    // Window initialization is needed
    this->Window.Initialize(true);
  }

  void LoadGeometry(const std::string& name, vtkAlgorithm* source, bool reset)
  {
    if (!this->DefaultScene || reset)
    {
      this->Reset();
    }

    // Manage progress bar
    vtkNew<vtkProgressBarWidget> progressWidget;
    vtkNew<vtkTimerLog> timer;
    loader_impl::internals::ProgressDataStruct callbackData;
    callbackData.timer = timer;
    callbackData.widget = progressWidget;
    const options_struct& optionsStruct = this->Options.getConstStruct();
    if (optionsStruct.ui.loader_progress && this->Interactor)
    {
      loader_impl::internals::CreateProgressRepresentationAndCallback(
        &callbackData, this->GenericImporter, this->Interactor);
    }

    // Add a single internal reader
    this->GenericImporter->AddInternalReader(name, source);

    // Update the importer
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    if (!this->GenericImporter->Update())
    {
      throw loader::load_failure_exception("failed to load geometry: " + name);
    }
#else
    this->GenericImporter->Update();
#endif

    // Remove anything progress related if any
    this->GenericImporter->RemoveObservers(vtkCommand::ProgressEvent);
    progressWidget->Off();

    // Initialize the animation using temporal information from the importer
    if (this->AnimationManager.Initialize(
          &this->Options, &this->Window, this->Interactor, this->GenericImporter))
    {
      double animationTime = optionsStruct.scene.animation.time;
      double timeRange[2];
      this->AnimationManager.GetTimeRange(timeRange);

      // We assume importers import data at timeRange[0] when not specified
      if (animationTime != timeRange[0])
      {
        this->AnimationManager.LoadAtTime(animationTime);
      }
    }

    // Set the name for animation
    this->Window.setAnimationNameInfo(this->AnimationManager.GetAnimationName());

    // Display the importer description
    loader_impl::internals::DisplayImporterDescription(this->GenericImporter);

    // Set the importer to use for coloring and actors
    this->Window.SetImporterForColoring(this->GenericImporter);

    // Initialize renderer and reset camera to bounds
    this->Window.UpdateDynamicOptions();
    this->Window.getCamera().resetToBounds();

    // Print info about scene and coloring
    this->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
    this->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

    this->DefaultScene = true;
  }

  bool DefaultScene = false;
  const options& Options;
  window_impl& Window;
  interactor_impl* Interactor = nullptr;
  animationManager AnimationManager;

  vtkSmartPointer<vtkImporter> CurrentFullSceneImporter;
  vtkNew<vtkF3DGenericImporter> GenericImporter;
};

//----------------------------------------------------------------------------
loader_impl::loader_impl(const options& options, window_impl& window)
  : Internals(std::make_unique<loader_impl::internals>(options, window))
{
  // Set render window on generic importer
  this->Internals->GenericImporter->SetRenderWindow(this->Internals->Window.GetRenderWindow());
}

//----------------------------------------------------------------------------
loader_impl::~loader_impl() = default;

//----------------------------------------------------------------------------
loader& loader_impl::loadGeometry(const std::string& filePath, bool reset)
{
  // Check file validity
  if (filePath.empty())
  {
    // Calling with reset is an usual codepath to reset the window
    if (!reset)
    {
      log::debug("Provided geometry file path is empty\n");
    }
    this->Internals->Reset();
    return *this;
  }
  if (!vtksys::SystemTools::FileExists(filePath, true))
  {
    throw loader::load_failure_exception(filePath + " does not exists");
  }

  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (reader)
  {
    log::debug("Found a reader for \"" + filePath + "\" : \"" + reader->getName() + "\"");
  }
  else
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported 3D geometry file format");
  }
  auto vtkReader = reader->createGeometryReader(filePath);
  if (!vtkReader)
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported 3D geometry file format for default scene");
  }
  // Read the file
  log::debug("Loading 3D geometry: ", filePath, "\n");

  this->Internals->LoadGeometry(vtksys::SystemTools::GetFilenameName(filePath), vtkReader, reset);

  return *this;
}

//----------------------------------------------------------------------------
loader& loader_impl::loadScene(const std::string& filePath)
{
  if (filePath.empty())
  {
    log::debug("No file to load a full scene provided\n");
    return *this;
  }
  if (!vtksys::SystemTools::FileExists(filePath, true))
  {
    throw loader::load_failure_exception(filePath + " does not exists");
  }

  // Recover the importer for the provided file path
  this->Internals->CurrentFullSceneImporter = nullptr;
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (reader)
  {
    log::debug("Found a reader for \"" + filePath + "\" : \"" + reader->getName() + "\"");
  }
  else
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported 3D scene file format");
  }
  this->Internals->CurrentFullSceneImporter = reader->createSceneReader(filePath);
  if (!this->Internals->CurrentFullSceneImporter)
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported 3D scene file format for full scene");
  }

  this->Internals->Window.Initialize(false);
  this->Internals->DefaultScene = false;

  // Initialize importer for rendering
  this->Internals->CurrentFullSceneImporter->SetRenderWindow(
    this->Internals->Window.GetRenderWindow());

  const options_struct& optionsStruct = this->Internals->Options.getConstStruct();
  int cameraIndex = optionsStruct.scene.camera.index;
  this->Internals->CurrentFullSceneImporter->SetCamera(cameraIndex);

  log::debug("Loading 3D scene: ", filePath, "\n");

  // Manage progress bar
  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;
  if (optionsStruct.ui.loader_progress && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->CurrentFullSceneImporter, this->Internals->Interactor);
  }

  // Read the file
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  if (!this->Internals->CurrentFullSceneImporter->Update())
  {
    throw loader::load_failure_exception("failed to load scene: " + filePath);
  }
#else
  this->Internals->CurrentFullSceneImporter->Update();
#endif

  // Remove anything progress related if any
  this->Internals->CurrentFullSceneImporter->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  // Initialize the animation using temporal information from the importer
  if (this->Internals->AnimationManager.Initialize(&this->Internals->Options,
        &this->Internals->Window, this->Internals->Interactor,
        this->Internals->CurrentFullSceneImporter))
  {
    double animationTime = optionsStruct.scene.animation.time;
    double timeRange[2];
    this->Internals->AnimationManager.GetTimeRange(timeRange);

    // We assume importers import data at timeRange[0] when not specified
    if (animationTime != timeRange[0])
    {
      this->Internals->AnimationManager.LoadAtTime(animationTime);
    }
  }

  // Set the name for animation
  this->Internals->Window.setAnimationNameInfo(
    this->Internals->AnimationManager.GetAnimationName());

  // Display output description
  loader_impl::internals::DisplayImporterDescription(this->Internals->CurrentFullSceneImporter);

  // Initialize renderer and reset camera to bounds if needed
  this->Internals->Window.UpdateDynamicOptions();
  if (cameraIndex == -1)
  {
    this->Internals->Window.getCamera().resetToBounds();
  }

  // Print info about scene and coloring
  this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
  this->Internals->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

  return *this;
}

//----------------------------------------------------------------------------
loader& loader_impl::loadGeometry(const mesh_t& mesh, bool reset)
{
  // sanity checks
  auto [valid, err] = mesh.isValid();
  if (!valid)
  {
    throw loader::load_failure_exception(err);
  }

  vtkNew<vtkF3DMemoryMesh> vtkSource;
  vtkSource->SetPoints(mesh.points);
  vtkSource->SetNormals(mesh.normals);
  vtkSource->SetTCoords(mesh.texture_coordinates);
  vtkSource->SetFaces(mesh.face_sides, mesh.face_indices);
  vtkSource->Update();

  this->Internals->LoadGeometry("<memory>", vtkSource, reset);

  return *this;
}

//----------------------------------------------------------------------------
bool loader_impl::hasSceneReader(const std::string& filePath)
{
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    return false;
  }
  return reader->hasSceneReader();
}

//----------------------------------------------------------------------------
bool loader_impl::hasGeometryReader(const std::string& filePath)
{
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    return false;
  }
  return reader->hasGeometryReader();
}

//----------------------------------------------------------------------------
void loader_impl::SetInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
  this->Internals->Interactor->SetAnimationManager(&this->Internals->AnimationManager);
}
}
