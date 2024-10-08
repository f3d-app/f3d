#include "scene_impl.h"

#include "animationManager.h"
#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "factory.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DMemoryMesh.h"
#include "vtkF3DMetaImporter.h"

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

    // Update the meta importer, the will only update importers that have not been update before
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    if (!this->MetaImporter->Update())
    {
      throw scene::load_failure_exception("failed to load scene");
    }
#else
    this->MetaImporter->Update();
#endif

    // Remove anything progress related if any
    this->MetaImporter->RemoveObservers(vtkCommand::ProgressEvent);
    progressWidget->Off();

    // Initialize the animation using temporal information from the importer
    if (this->AnimationManager.Initialize())
    {
      if (this->Options.scene.animation.time.has_value())
      {
        double animationTime = this->Options.scene.animation.time.value();
        double timeRange[2];
        this->AnimationManager.GetTimeRange(timeRange);

        // We assume importers import data at timeRange[0] when not specified
        if (animationTime != timeRange[0])
        {
          this->AnimationManager.LoadAtTime(animationTime);
        }
      }
    }

    // Set the name for animation
    this->Window.setAnimationNameInfo(this->AnimationManager.GetAnimationName());

    // Display output description
    scene_impl::internals::DisplayImporterDescription(this->MetaImporter);

    // Update all window options and reset camera to bounds if needed
    this->Window.UpdateDynamicOptions();
    if (!this->Options.scene.camera.index.has_value())
    {
      this->Window.getCamera().resetToBounds();
    }

    // Print info about scene and coloring
    this->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
    this->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);
  }

  static void DisplayImporterDescription(vtkImporter* importer)
  {
    vtkIdType availCameras = importer->GetNumberOfCameras();
    if (availCameras <= 0)
    {
      log::debug("No camera available");
    }
    else
    {
      log::debug("Camera(s) available are:");
    }
    for (int i = 0; i < availCameras; i++)
    {
      log::debug(i, ": ", importer->GetCameraName(i));
    }
    log::debug("");
    log::debug(importer->GetOutputsDescription(), "\n");
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
    f3d::reader* reader = f3d::factory::instance()->getReader(filePath.string());
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
      auto vtkReader = reader->createGeometryReader(filePath.string());
      // XXX: We assume the f3d reader provide either a scene reader or a geometry reader
      // TODO: Put in f3d::reader directly
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
  return f3d::factory::instance()->getReader(filePath.string()) != nullptr;
}

//----------------------------------------------------------------------------
void scene_impl::SetInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
  this->Internals->AnimationManager.SetInteractor(interactor);
  this->Internals->Interactor->SetAnimationManager(&this->Internals->AnimationManager);
}
}
