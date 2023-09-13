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

  void LoadGeometry(const std::string& name, vtkAlgorithm* source, bool reset)
  {
    if (!this->DefaultScene || reset)
    {
      // Reset the generic importer
      this->GenericImporter->RemoveInternalReaders();

      // Remove the importer from the renderer
      this->Window.SetImporterForColoring(nullptr);

      // Window initialization is needed
      this->Window.Initialize(true);
    }

    // Manage progress bar
    vtkNew<vtkProgressBarWidget> progressWidget;
    vtkNew<vtkTimerLog> timer;
    loader_impl::internals::ProgressDataStruct callbackData;
    callbackData.timer = timer;
    callbackData.widget = progressWidget;
    if (this->Options.getAsBool("ui.loader-progress") && this->Interactor)
    {
      loader_impl::internals::CreateProgressRepresentationAndCallback(
        &callbackData, this->GenericImporter, this->Interactor);
    }

    // Add a single internal reader
    this->GenericImporter->AddInternalReader(name, source);

    // Update the importer
    this->GenericImporter->Update();

    // Remove anything progress related if any
    this->GenericImporter->RemoveObservers(vtkCommand::ProgressEvent);
    progressWidget->Off();

    // Initialize the animation using temporal information from the importer
    this->AnimationManager.Initialize(
      &this->Options, &this->Window, this->Interactor, this->GenericImporter);

    double animationTime = this->Options.getAsDouble("scene.animation.time");
    if (animationTime != 0)
    {
      this->AnimationManager.LoadAtTime(animationTime);
    }

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
loader& loader_impl::loadGeometry(const mesh_t& mesh, bool reset)
{
  // sanity checks
  if (mesh.points.size() == 0)
  {
    throw loader::load_failure_exception("The points buffer must not be empty.");
  }

  if (mesh.points.size() % 3 != 0)
  {
    throw loader::load_failure_exception(
      "The points buffer is not a multiple of 3. It's length is " +
      std::to_string(mesh.points.size()));
  }

  size_t nbPoints = mesh.points.size() / 3;

  if (mesh.normals.size() > 0 && mesh.normals.size() != nbPoints * 3)
  {
    throw loader::load_failure_exception(
      "The normals buffer must be empty or equal to 3 times the number of points.");
  }

  if (mesh.texture_coordinates.size() > 0 && mesh.texture_coordinates.size() != nbPoints * 2)
  {
    throw loader::load_failure_exception(
      "The texture_coordinates buffer must be empty or equal to 2 times the number of points.");
  }

  unsigned int expectedSize = 0;
  for (unsigned int currentSize : mesh.face_sides)
  {
    expectedSize += currentSize;
  }

  if (mesh.face_indices.size() != expectedSize)
  {
    throw loader::load_failure_exception(
      "The face_indices buffer size is invalid, it should be " + std::to_string(expectedSize));
  }

  auto it = std::find_if(mesh.face_indices.cbegin(), mesh.face_indices.cend(),
    [=](unsigned int idx) { return idx >= nbPoints; });
  if (it != mesh.face_indices.cend())
  {
    throw loader::load_failure_exception("Face vertex at index " +
      std::to_string(std::distance(mesh.face_indices.cbegin(), it)) +
      " is greater than the maximum vertex index (" + std::to_string(nbPoints) + ")");
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
