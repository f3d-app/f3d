#include "loader_impl.h"

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

  typedef struct ProgressDataStruct
  {
    vtkTimerLog* timer;
    vtkProgressBarWidget* widget;
  } ProgressDataStruct;

  static vtkSmartPointer<vtkImporter> GetFullSceneImporter(const std::string& fileName)
  {
    // Find the best compatible reader with scene reading capabilities based on reader scores
    f3d::reader* reader = f3d::factory::instance()->getReader(fileName);
    if (!reader)
    {
      return nullptr;
    }

    return reader->createSceneReader(fileName);
  }

/*  static vtkSmartPointer<vtkImporter> GetImporter(const std::string& fileName, bool geometry)
  {
    // Find the best compatible reader with scene reading capabilities based on reader scores
    f3d::reader* reader = f3d::factory::instance()->getReader(fileName);

    if (!reader)
    {
      return nullptr;
    }

    if (!geometry)
    {
      vtkSmartPointer<vtkImporter> importer = reader->createSceneReader(fileName);
      if (importer)
      {
        return importer;
      }
    }

    // Use the generic importer and check if it can process the file
    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader->createGeometryReader(fileName));
    if (!importer->CanReadFile())
    {
      return nullptr;
    }
    return importer;
  }*/

  static void InitializeImporterWithOptions(const options& options, vtkF3DGenericImporter* importer)
  {
    // Move to interactor TODO
    importer->SetSurfaceColor(options.getAsDoubleVector("model.color.rgb").data());
    importer->SetOpacity(options.getAsDouble("model.color.opacity"));
    importer->SetTextureBaseColor(options.getAsString("model.color.texture"));

    importer->SetRoughness(options.getAsDouble("model.material.roughness"));
    importer->SetMetallic(options.getAsDouble("model.material.metallic"));
    importer->SetTextureMaterial(options.getAsString("model.material.texture"));

    importer->SetTextureEmissive(options.getAsString("model.emissive.texture"));
    importer->SetEmissiveFactor(options.getAsDoubleVector("model.emissive.factor").data());

    importer->SetTextureNormal(options.getAsString("model.normal.texture"));
    importer->SetNormalScale(options.getAsDouble("model.normal.scale"));
  }

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
        if (progressData->timer->GetElapsedTime() > 0.15)
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

  bool DefaultScene = true;
  std::string FilenameInfo;

  const options& Options;
  window_impl& Window;
  interactor_impl* Interactor = nullptr;

  vtkSmartPointer<vtkImporter> CurrentImporter; // TODO may not be needed
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
/*bool loader_impl::loadFile(const std::string& filePath)
{
  // Recover fileNameInfo if any
  std::string fileNameInfo = this->Internals->FilenameInfo;
  if (fileNameInfo.empty())
  {
    fileNameInfo = vtksys::SystemTools::GetFilenameName(filePath);
  }

  if (filePath.empty())
  {
    // No file provided, show a drop zone instead
    log::debug("No file to load provided\n");
    this->Internals->Window.Initialize(
      false, fileNameInfo + "No file to load provided, please drop one into this window");
    return false;
  }

  // There is a file to load, update CurrentFileIndex
  log::debug("Loading: ", filePath, "\n");

  // Recover the importer
  this->Internals->CurrentImporter = loader_impl::internals::GetImporter(
    filePath, this->Internals->Options.getAsBool("scene.geometry-only"));
  vtkF3DGenericImporter* genericImporter =
    vtkF3DGenericImporter::SafeDownCast(this->Internals->CurrentImporter);
  if (!this->Internals->CurrentImporter)
  {
    log::warn(filePath, " is not a file of a supported file format\n");
    this->Internals->Window.Initialize(false, fileNameInfo + " [UNSUPPORTED]");
    return false;
  }

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;

  this->Internals->Window.Initialize(genericImporter != nullptr, fileNameInfo);

  // Initialize importer for rendering
  this->Internals->CurrentImporter->SetRenderWindow(this->Internals->Window.GetRenderWindow());

  int cameraIndex = this->Internals->Options.getAsInt("scene.camera.index");
// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210303)
  this->Internals->CurrentImporter->SetCamera(cameraIndex);
#else
  // XXX There is no way to recover the init value yet, assume it is -1
  if (cameraIndex != -1)
  {
    log::warn("This VTK version does not support specifying the camera index, ignored.");
  }
#endif

  // Manage progress bar
  if (this->Internals->Options.getAsBool("ui.loader-progress") && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->CurrentImporter, this->Internals->Interactor);
  }

  // Initialize genericImporter with options
  if (genericImporter)
  {
    loader_impl::internals::InitializeImporterWithOptions(
      this->Internals->Options, genericImporter);
  }

  // Read the file
  this->Internals->CurrentImporter->Update();
  loader_impl::internals::DisplayImporterDescription(this->Internals->CurrentImporter);

  // Remove anything progress related if any
  this->Internals->CurrentImporter->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  if (this->Internals->Interactor)
  {
    // Initialize the animation using temporal information from the importer
    this->Internals->Interactor->InitializeAnimation(this->Internals->CurrentImporter);
  }

  // Recover generic importer specific actors and mappers to set on the renderer with coloring
  if (genericImporter)
  {
    this->Internals->Window.InitializeRendererWithColoring(genericImporter);
  }

  // Initialize renderer and reset camera to bounds if needed
  this->Internals->Window.UpdateDynamicOptions();
  if (cameraIndex == -1)
  {
    this->Internals->Window.getCamera().resetToBounds();
  }

  // Print info about scene and coloring
  this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
  this->Internals->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

  return true;
  ;
}*/

//----------------------------------------------------------------------------
loader& loader_impl::setFilenameInfo(const std::string& filenameInfo)
{
  this->Internals->FilenameInfo = filenameInfo;
  return *this;
}

//----------------------------------------------------------------------------
void loader_impl::setInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
}

//----------------------------------------------------------------------------
bool loader_impl::addGeometry(const std::string& filePath)
{
  if (!this->Internals->DefaultScene)
  {
    // TODO
    log::warn("Cannot add a geometry as default scene is not currently in use\n");
    return false;
  }

  // Read the file
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    log::warn(filePath, " is not a file of a supported file format\n");
    return false;
  }

  this->Internals->GenericImporter->AddInternalReader(reader->createGeometryReader(filePath));
  this->Internals->GenericImporter->Update();
  loader_impl::internals::DisplayImporterDescription(this->Internals->CurrentImporter);

  if (this->Internals->Interactor)
  {
    // Initialize the animation using temporal information from the importer
    this->Internals->Interactor->InitializeAnimation(this->Internals->CurrentImporter);
  }

  // Recover generic importer specific actors and mappers to set on the renderer with coloring
  this->Internals->Window.InitializeRendererWithColoring(this->Internals->GenericImporter);

  // Initialize renderer and reset camera to bounds
  this->Internals->Window.UpdateDynamicOptions();
  this->Internals->Window.getCamera().resetToBounds();

  // Print info about scene and coloring
  this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
  this->Internals->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

  return true;
}

//----------------------------------------------------------------------------
loader& loader_impl::resetToDefaultScene()
{
  // Recover the generic importer
  this->Internals->GenericImporter->RemoveInternalReaders();
  this->Internals->CurrentImporter = this->Internals->GenericImporter;

  this->Internals->Window.Initialize(true);

  // Initialize genericImporter with options
  loader_impl::internals::InitializeImporterWithOptions(
    this->Internals->Options, this->Internals->GenericImporter);

  this->Internals->DefaultScene = true;

  return *this;
}

//----------------------------------------------------------------------------
bool loader_impl::loadFullScene(const std::string& filePath)
{

  // TODO what if it is empty ?

  // There is a file to load, update CurrentFileIndex
  log::debug("Loading full scene: ", filePath, "\n");

  // Recover the importer
  this->Internals->CurrentImporter = loader_impl::internals::GetFullSceneImporter(filePath);
  if (!this->Internals->CurrentImporter)
  {
    log::warn(filePath, " is not a file of a supported file format for full scene\n");
    this->Internals->Window.Initialize(false);
    return false;
  }

  // TODO generify ?
  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;

  this->Internals->Window.Initialize(false);

  // Initialize importer for rendering
  this->Internals->CurrentImporter->SetRenderWindow(this->Internals->Window.GetRenderWindow());

  int cameraIndex = this->Internals->Options.getAsInt("scene.camera.index");
// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210303)
  this->Internals->CurrentImporter->SetCamera(cameraIndex);
#else
  // XXX There is no way to recover the init value yet, assume it is -1
  if (cameraIndex != -1)
  {
    log::warn("This VTK version does not support specifying the camera index, ignored.");
  }
#endif

  // Manage progress bar
  if (this->Internals->Options.getAsBool("ui.loader-progress") && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->CurrentImporter, this->Internals->Interactor);
  }

  // Read the file
  this->Internals->CurrentImporter->Update();
  loader_impl::internals::DisplayImporterDescription(this->Internals->CurrentImporter);

  // Remove anything progress related if any
  this->Internals->CurrentImporter->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  if (this->Internals->Interactor)
  {
    // Initialize the animation using temporal information from the importer
    this->Internals->Interactor->InitializeAnimation(this->Internals->CurrentImporter);
  }

  // Initialize renderer and reset camera to bounds if needed
  this->Internals->Window.UpdateDynamicOptions();
  if (cameraIndex == -1)
  {
    this->Internals->Window.getCamera().resetToBounds();
  }

  // Print info about scene and coloring
  this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
  this->Internals->Window.PrintSceneDescription(log::VerboseLevel::DEBUG);

  this->Internals->DefaultScene = false;
  return true;
}

//----------------------------------------------------------------------------
bool loader_impl::canReadScene(const std::string& filePath)
{
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    return false;
  }
  return reader->hasSceneReader();
}

//----------------------------------------------------------------------------
bool loader_impl::canReadGeometry(const std::string& filePath)
{
  f3d::reader* reader = f3d::factory::instance()->getReader(filePath);
  if (!reader)
  {
    return false;
  }
  return reader->hasGeometryReader();
}
}
