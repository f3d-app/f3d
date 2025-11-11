#include "scene_impl.h"

#include "animationManager.h"
#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "scene.h"
#include "window_impl.h"

#include "F3DStyle.h"
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

  static void CreateProgressRepresentationAndCallback(ProgressDataStruct* data,
    vtkImporter* importer, interactor_impl* interactor, const f3d::color_t& color)
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
    progressRep->SetProgressBarColor(color.r(), color.g(), color.b());
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
      f3d::color_t color;
      if (!this->Options.ui.loader_progress_color.has_value())
      {
        const auto [r, g, b] = F3DStyle::GetF3DYellow();
        color = color_t(r, g, b);
      }
      else
      {
        color = this->Options.ui.loader_progress_color.value();
      }
      scene_impl::internals::CreateProgressRepresentationAndCallback(
        &callbackData, this->MetaImporter, this->Interactor, color);
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
int scene_impl::addLight(const light_state_t& lightState) const
{
  vtkNew<vtkLight> newLight;
  newLight->SetLightType(static_cast<int>(lightState.type));
  newLight->SetPosition(lightState.position.data());
  newLight->SetColor(lightState.color.data());
  newLight->SetPositional(lightState.positionalLight);
  newLight->SetFocalPoint(lightState.position[0] + lightState.direction[0],
    lightState.position[1] + lightState.direction[1],
    lightState.position[2] + lightState.direction[2]);
  newLight->SetIntensity(lightState.intensity);
  newLight->SetSwitch(lightState.switchState);
  this->Internals->Window.GetRenderer()->AddLight(newLight);
  return this->getLightCount() - 1;
}

//----------------------------------------------------------------------------
int scene_impl::getLightCount() const
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  return lc->GetNumberOfItems();
}

//----------------------------------------------------------------------------
light_state_t scene_impl::getLight(int index) const
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  vtkLight* light = vtkLight::SafeDownCast(lc->GetItemAsObject(index));
  if (!light)
  {
    throw scene::light_exception("No light at index " + std::to_string(index) + " to get");
  }

  const double* position = light->GetPosition();
  const double* color = light->GetDiffuseColor();
  const double* focalPoint = light->GetFocalPoint();

  light_state_t lightState;
  lightState.type = static_cast<light_type>(light->GetLightType());
  lightState.position = { position[0], position[1], position[2] };
  lightState.color = { color[0], color[1], color[2] };
  lightState.direction = { focalPoint[0] - position[0], focalPoint[1] - position[1],
    focalPoint[2] - position[2] };
  lightState.positionalLight = light->GetPositional();
  lightState.intensity = light->GetIntensity();
  lightState.switchState = light->GetSwitch();
  return lightState;
}

//----------------------------------------------------------------------------
scene& scene_impl::updateLight(int index, const light_state_t& lightState)
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  vtkLight* light = vtkLight::SafeDownCast(lc->GetItemAsObject(index));
  if (!light)
  {
    throw scene::light_exception("No light at index " + std::to_string(index) + " to update");
  }

  light->SetLightType(static_cast<int>(lightState.type));
  light->SetPosition(lightState.position.data());
  light->SetColor(lightState.color.data());
  light->SetPositional(lightState.positionalLight);
  light->SetFocalPoint(lightState.position[0] + lightState.direction[0],
    lightState.position[1] + lightState.direction[1],
    lightState.position[2] + lightState.direction[2]);
  light->SetIntensity(lightState.intensity);
  light->SetSwitch(lightState.switchState);

  return *this;
}

//----------------------------------------------------------------------------
scene& scene_impl::removeLight(int index)
{
  vtkLightCollection* lc = this->Internals->Window.GetRenderer()->GetLights();
  vtkLight* light = vtkLight::SafeDownCast(lc->GetItemAsObject(index));
  if (!light)
  {
    throw scene::light_exception("No light at index " + std::to_string(index) + " to remove");
  }

  this->Internals->Window.GetRenderer()->RemoveLight(light);
  return *this;
}

//----------------------------------------------------------------------------
scene& scene_impl::removeAllLights()
{
  this->Internals->Window.GetRenderer()->RemoveAllLights();
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
