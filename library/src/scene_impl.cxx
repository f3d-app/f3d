#include "scene_impl.h"

#include "animationManager.h"
#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "scene.h"
#include "window_impl.h"

#include "factory.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DMemoryMesh.h"
#include "vtkF3DMetaImporter.h"

#include <optional>
#include <vtkCallbackCommand.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <algorithm>
#include <numeric>
#include <vector>

namespace fs = std::filesystem;

namespace f3d::detail
{
class scene_impl::internals
{
public:
  internals(const options& options, window_impl& window)
    : Options(options)
    , Window(window)
    , AnimationManager(options, window)
  {
    this->MetaImporter->SetRenderWindow(this->Window.GetRenderWindow());
    this->Window.SetImporter(this->MetaImporter);
    this->AnimationManager.SetImporter(this->MetaImporter);
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

  void Load(const std::vector<vtkSmartPointer<vtkImporter>>& importers)
  {
    for (const vtkSmartPointer<vtkImporter>& importer : importers)
    {
      this->MetaImporter->AddImporter(importer);
    }

    // Initialize the UpVector on load
    this->Window.InitializeUpVector();

    // Reset temporary up to apply any config values
    if (this->Interactor)
    {
      this->Interactor->ResetTemporaryUp();
    }

    if (this->Options.scene.camera.index.has_value())
    {
      this->MetaImporter->SetCameraIndex(this->Options.scene.camera.index.value());
    }

    // Manage progress bar
    vtkNew<vtkProgressBarWidget> progressWidget;
    vtkNew<vtkTimerLog> timer;
    scene_impl::internals::ProgressDataStruct callbackData;
    callbackData.timer = timer;
    callbackData.widget = progressWidget;
    if (this->Options.ui.loader_progress && this->Interactor)
    {
      scene_impl::internals::CreateProgressRepresentationAndCallback(
        &callbackData, this->MetaImporter, this->Interactor);
    }

    // Update the meta importer, the will only update importers that have not been updated before
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    if (!this->MetaImporter->Update())
    {
      this->MetaImporter->Clear();
      this->Window.Initialize();
      throw scene::load_failure_exception("failed to load scene");
    }
#else
    this->MetaImporter->Update();
#endif

    // Remove anything progress related if any
    this->MetaImporter->RemoveObservers(vtkCommand::ProgressEvent);
    progressWidget->Off();

    // Initialize the animation using temporal information from the importer
    this->AnimationManager.Initialize();

    // Update all window options and reset camera to bounds if needed
    this->Window.UpdateDynamicOptions();
    if (!this->Options.scene.camera.index.has_value())
    {
      this->Window.getCamera().resetToBounds();
    }

    scene_impl::internals::DisplayAllInfo(this->MetaImporter, this->Window);
  }

  static void DisplayImporterDescription(log::VerboseLevel level, vtkImporter* importer)
  {
    vtkIdType availCameras = importer->GetNumberOfCameras();
    if (availCameras <= 0)
    {
      log::print(level, "No camera available");
    }
    else
    {
      log::print(level, "Camera(s) available are:");
    }
    for (int i = 0; i < availCameras; i++)
    {
      log::print(level, i, ": ", importer->GetCameraName(i));
    }
    log::print(level, "");
    log::print(level, importer->GetOutputsDescription(), "\n");
  }

  static void DisplayAllInfo(vtkImporter* importer, window_impl& window)
  {
    // Display output description
    scene_impl::internals::DisplayImporterDescription(log::VerboseLevel::DEBUG, importer);

    // Display coloring information
    window.PrintColoringDescription(log::VerboseLevel::DEBUG);
    log::debug("");

    // Print scene description
    window.PrintSceneDescription(log::VerboseLevel::DEBUG);
  }

  const options& Options;
  window_impl& Window;
  interactor_impl* Interactor = nullptr;
  animationManager AnimationManager;

  vtkNew<vtkF3DMetaImporter> MetaImporter;
};

//----------------------------------------------------------------------------
scene_impl::scene_impl(const options& options, window_impl& window)
  : Internals(std::make_unique<scene_impl::internals>(options, window))
{
}

//----------------------------------------------------------------------------
scene_impl::~scene_impl() = default;

//----------------------------------------------------------------------------
scene& scene_impl::add(const fs::path& filePath)
{
  std::vector<fs::path> paths = { filePath };
  return this->add(paths);
}

//----------------------------------------------------------------------------
scene& scene_impl::add(const std::vector<std::string>& filePathStrings)
{
  std::vector<fs::path> paths;
  paths.reserve(filePathStrings.size());
  for (const std::string& str : filePathStrings)
  {
    paths.emplace_back(str);
  }
  return this->add(paths);
}

//----------------------------------------------------------------------------
scene& scene_impl::add(const std::vector<fs::path>& filePaths)
{
  if (filePaths.empty())
  {
    log::debug("No file to load a full scene provided\n");
    return *this;
  }

  std::vector<vtkSmartPointer<vtkImporter>> importers;
  for (const fs::path& filePath : filePaths)
  {
    if (filePath.empty())
    {
      log::debug("An empty file to load was provided\n");
      continue;
    }
    if (!vtksys::SystemTools::FileExists(filePath.string(), true))
    {
      throw scene::load_failure_exception(filePath.string() + " does not exists");
    }

    // Recover the importer for the provided file path
    f3d::reader* reader = f3d::factory::instance()->getReader(
      filePath.string(), this->Internals->Options.scene.force_reader);
    if (reader)
    {
      log::debug(
        "Found a reader for \"" + filePath.string() + "\" : \"" + reader->getName() + "\"");
    }
    else
    {
      throw scene::load_failure_exception(
        filePath.string() + " is not a file of a supported 3D scene file format");
    }
    vtkSmartPointer<vtkImporter> importer = reader->createSceneReader(filePath.string());
    if (!importer)
    {
      // XXX: F3D Plugin CMake logic ensure there is either a scene reader or a geometry reader
      auto vtkReader = reader->createGeometryReader(filePath.string());
      assert(vtkReader);
      vtkSmartPointer<vtkF3DGenericImporter> genericImporter =
        vtkSmartPointer<vtkF3DGenericImporter>::New();
      genericImporter->SetInternalReader(vtkReader);
      importer = genericImporter;
    }
    importers.emplace_back(importer);
  }

  log::debug("\nLoading files: ");
  if (filePaths.size() == 1)
  {
    log::debug(filePaths[0].string());
  }
  else
  {
    for (const fs::path& filePathStr : filePaths)
    {
      log::debug("- ", filePathStr.string());
    }
  }
  log::debug("");

  this->Internals->Load(importers);
  return *this;
}

//----------------------------------------------------------------------------
scene& scene_impl::add(const mesh_t& mesh)
{
  // sanity checks
  auto [valid, err] = mesh.isValid();
  if (!valid)
  {
    throw scene::load_failure_exception(err);
  }

  vtkNew<vtkF3DMemoryMesh> vtkSource;
  vtkSource->SetPoints(mesh.points);
  vtkSource->SetNormals(mesh.normals);
  vtkSource->SetTCoords(mesh.texture_coordinates);
  vtkSource->SetFaces(mesh.face_sides, mesh.face_indices);

  vtkSmartPointer<vtkF3DGenericImporter> importer = vtkSmartPointer<vtkF3DGenericImporter>::New();
  importer->SetInternalReader(vtkSource);

  log::debug("Loading 3D scene from memory");
  this->Internals->Load({ importer });
  return *this;
}

//----------------------------------------------------------------------------
scene& scene_impl::clear()
{
  // Clear the meta importer from all importers
  this->Internals->MetaImporter->Clear();

  // Clear the window of all actors
  this->Internals->Window.Initialize();

  return *this;
}

//----------------------------------------------------------------------------
bool scene_impl::supports(const fs::path& filePath)
{
  return f3d::factory::instance()->getReader(
           filePath.string(), this->Internals->Options.scene.force_reader) != nullptr;
}

//----------------------------------------------------------------------------
scene& scene_impl::loadAnimationTime(double timeValue)
{
  this->Internals->AnimationManager.LoadAtTime(timeValue);
  scene_impl::internals::DisplayAllInfo(this->Internals->MetaImporter, this->Internals->Window);
  return *this;
}

//----------------------------------------------------------------------------
std::pair<double, double> scene_impl::animationTimeRange()
{
  return this->Internals->AnimationManager.GetTimeRange();
}

//----------------------------------------------------------------------------
void scene_impl::SetInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
  this->Internals->AnimationManager.SetInteractor(interactor);
  this->Internals->Interactor->SetAnimationManager(&this->Internals->AnimationManager);
}

void scene_impl::PrintImporterDescription(log::VerboseLevel level)
{
  scene_impl::internals::DisplayImporterDescription(level, this->Internals->MetaImporter);
}
}
