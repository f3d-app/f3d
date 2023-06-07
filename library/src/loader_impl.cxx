#include "loader_impl.h"

#include "animationManager.h"
#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "factory.h"
#include "vtkF3DGenericImporter.h"

#include <vtkCallbackCommand.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <algorithm>
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
          vtksys::SystemTools::HasEnv("CTEST_PROGRESS_BAR_TESTING"))
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

// Complete vtkProgressBarRepresentation needs
// https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7359
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201027)
    progressRep->DrawFrameOff();
    progressRep->SetPadding(0.0, 0.0);
#endif
    data->timer->StartTimer();
  }

  static void DisplayImporterDescription(vtkImporter* importer)
  {
// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210303)
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
#endif
    log::debug(importer->GetOutputsDescription(), "\n");
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
  if (!this->Internals->DefaultScene || reset)
  {
    // Reset the generic importer
    this->Internals->GenericImporter->RemoveInternalReaders();

    // Remove the importer from the renderer
    this->Internals->Window.SetImporterForColoring(nullptr);

    // Window initialization is needed
    this->Internals->Window.Initialize(true);
  }

  // Check file validity
  if (filePath.empty())
  {
    // Calling with reset is an usual codepath to reset the window
    if (!reset)
    {
      log::debug("Provided geometry file path is empty\n");
    }
    return *this;
  }
  if (!vtksys::SystemTools::FileExists(filePath, true))
  {
    throw loader::load_failure_exception(filePath + " does not exists");
  }

  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    throw loader::load_failure_exception(filePath + " is not a file of a supported file format");
  }
  auto vtkReader = reader->createGeometryReader(filePath);
  if (!vtkReader)
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported file format for default scene");
  }

  // Read the file
  log::debug("Loading: ", filePath, "\n");

  // Manage progress bar
  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;
  if (this->Internals->Options.getAsBool("ui.loader-progress") && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->GenericImporter, this->Internals->Interactor);
  }

  // Add a single internal reader
  this->Internals->GenericImporter->AddInternalReader(
    vtksys::SystemTools::GetFilenameName(filePath), vtkReader);

  // Update the importer
  this->Internals->GenericImporter->Update();

  // Remove anything progress related if any
  this->Internals->GenericImporter->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  // Initialize the animation using temporal information from the importer
  this->Internals->AnimationManager.Initialize(&this->Internals->Options, &this->Internals->Window,
    this->Internals->Interactor, this->Internals->GenericImporter);

  double animationTime = this->Internals->Options.getAsDouble("scene.animation.time");
  if (animationTime != 0)
  {
    this->Internals->AnimationManager.LoadAtTime(animationTime);
  }

  // Display the importer description
  loader_impl::internals::DisplayImporterDescription(this->Internals->GenericImporter);

  // Set the importer to use for coloring and actors
  this->Internals->Window.SetImporterForColoring(this->Internals->GenericImporter);

  // Initialize renderer and reset camera to bounds
  this->Internals->Window.UpdateDynamicOptions();
  this->Internals->Window.getCamera().resetToBounds();

  // Print info about scene and coloring
  this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
  this->Internals->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

  this->Internals->DefaultScene = true;

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
  if (!reader)
  {
    throw loader::load_failure_exception(filePath + " is not a file of a supported file format");
  }
  this->Internals->CurrentFullSceneImporter = reader->createSceneReader(filePath);
  if (!this->Internals->CurrentFullSceneImporter)
  {
    throw loader::load_failure_exception(
      filePath + " is not a file of a supported file format for full scene");
  }

  this->Internals->Window.Initialize(false);
  this->Internals->DefaultScene = false;

  // Initialize importer for rendering
  this->Internals->CurrentFullSceneImporter->SetRenderWindow(
    this->Internals->Window.GetRenderWindow());

  int cameraIndex = this->Internals->Options.getAsInt("scene.camera.index");
// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210303)
  this->Internals->CurrentFullSceneImporter->SetCamera(cameraIndex);
#else
  // XXX There is no way to recover the init value yet, assume it is -1
  if (cameraIndex != -1)
  {
    log::warn("This VTK version does not support specifying the camera index, ignored.");
  }
#endif

  log::debug("Loading full scene: ", filePath, "\n");

  // Manage progress bar
  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;
  if (this->Internals->Options.getAsBool("ui.loader-progress") && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->CurrentFullSceneImporter, this->Internals->Interactor);
  }

  // Read the file
  this->Internals->CurrentFullSceneImporter->Update();

  // Remove anything progress related if any
  this->Internals->CurrentFullSceneImporter->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  // Initialize the animation using temporal information from the importer
  this->Internals->AnimationManager.Initialize(&this->Internals->Options, &this->Internals->Window,
    this->Internals->Interactor, this->Internals->CurrentFullSceneImporter);

  double animationTime = this->Internals->Options.getAsDouble("scene.animation.time");
  if (animationTime != 0)
  {
    this->Internals->AnimationManager.LoadAtTime(animationTime);
  }

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
