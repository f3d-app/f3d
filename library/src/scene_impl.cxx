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
#include "vtkF3DRenderer.h"

#include <optional>
#include <vtkCallbackCommand.h>
#include <vtkLightCollection.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <vector>

namespace fs = std::filesystem;

namespace f3d::detail
{
class scene_impl::internals
{
public:
  internals(options& options, window_impl& window)
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
      this->MetaImporter->RemoveObservers(vtkCommand::ProgressEvent);
      progressWidget->Off();

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
scene_impl::scene_impl(options& options, window_impl& window)
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
  std::vector<fs::path> paths(filePathStrings.size());
  std::copy(filePathStrings.begin(), filePathStrings.end(), paths.begin());
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
    std::optional<std::string> forceReader = this->Internals->Options.scene.force_reader;
    // Recover the importer for the provided file path
    const f3d::reader* reader = f3d::factory::instance()->getReader(filePath.string(), forceReader);
    if (reader)
    {
      if (forceReader)
      {
        log::debug("Forcing reader ", (*forceReader), " for ", filePath.string());
      }
      else
      {
        log::debug("Found a reader for \"", filePath.string(), "\" : \"", reader->getName(), "\"");
      }
    }
    else
    {
      if (forceReader)
      {
        throw scene::load_failure_exception(*forceReader + " is not a valid force reader");
      }
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
void scene_impl::addLight(const light_state_t& lightState)
{
  vtkNew<vtkLight> vtkLight;
  vtkLight->SetLightType(static_cast<int>(lightState.type));
  vtkLight->SetPosition(lightState.position[0], lightState.position[1], lightState.position[2]);
  vtkLight->SetColor(lightState.color[0], lightState.color[1], lightState.color[2]);
  vtkLight->SetPositional(lightState.positionalLight);
  vtkLight->SetFocalPoint(lightState.position[0] + lightState.direction[0],
    lightState.position[1] + lightState.direction[1],
    lightState.position[2] + lightState.direction[2]);
  vtkLight->SetIntensity(lightState.intensity);
  this->Internals->Window.GetRenderer()->AddLight(vtkLight);
}

//----------------------------------------------------------------------------
int scene_impl::getLightCount() const
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  lc->InitTraversal();

  int lightCount = 0;
  for (vtkLight* vtkL = lc->GetNextItem(); vtkL != nullptr; vtkL = lc->GetNextItem())
  {
    if (vtkL->GetSwitch())
    {
      lightCount++;
    }
  }
  return lightCount;
}

//----------------------------------------------------------------------------
light_state_t scene_impl::getLight(int index)
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  lc->InitTraversal();

  int lightCount = 0;
  for (vtkLight* vtkL = lc->GetNextItem(); vtkL != nullptr; vtkL = lc->GetNextItem())
  {
    if (vtkL->GetSwitch())
    {
      if (lightCount == index)
      {
        light_state_t lightState;
        lightState.type = static_cast<light_type>(vtkL->GetLightType());
        lightState.position = { vtkL->GetPosition()[0], vtkL->GetPosition()[1],
          vtkL->GetPosition()[2] };
        lightState.color = { vtkL->GetDiffuseColor()[0], vtkL->GetDiffuseColor()[1],
          vtkL->GetDiffuseColor()[2] };
        lightState.direction = { vtkL->GetFocalPoint()[0] - vtkL->GetPosition()[0],
          vtkL->GetFocalPoint()[1] - vtkL->GetPosition()[1],
          vtkL->GetFocalPoint()[2] - vtkL->GetPosition()[2] };
        lightState.positionalLight = vtkL->GetPositional();
        lightState.intensity = vtkL->GetIntensity();
        return lightState;
      }
      lightCount++;
    }
  }
  log::warn("No light at index ", index, " to get");
  return light_state_t{};
}

//----------------------------------------------------------------------------
void scene_impl::updateLight(int index, const light_state_t& lightState)
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  lc->InitTraversal();

  int lightCount = 0;
  for (vtkLight* vtkL = lc->GetNextItem(); vtkL != nullptr; vtkL = lc->GetNextItem())
  {
    if (vtkL->GetSwitch())
    {
      if (lightCount == index)
      {
        vtkL->SetLightType(static_cast<int>(lightState.type));
        vtkL->SetPosition(lightState.position[0], lightState.position[1], lightState.position[2]);
        vtkL->SetColor(lightState.color[0], lightState.color[1], lightState.color[2]);
        vtkL->SetPositional(lightState.positionalLight);
        vtkL->SetFocalPoint(lightState.position[0] + lightState.direction[0],
          lightState.position[1] + lightState.direction[1],
          lightState.position[2] + lightState.direction[2]);
        vtkL->SetIntensity(lightState.intensity);
        return;
      }
      lightCount++;
    }
  }
  log::warn("No light at index ", index, " to update");
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
unsigned int scene_impl::availableAnimations() const
{
  return this->Internals->AnimationManager.GetNumberOfAvailableAnimations();
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
