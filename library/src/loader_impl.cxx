#include "loader_impl.h"

#include "interactor_impl.h"
#include "log.h"
#include "options.h"
#include "window_impl.h"

#include "F3DReaderFactory.h"
#include "vtkF3DGenericImporter.h"

#include <vtkCallbackCommand.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/Directory.hxx>
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

  static vtkSmartPointer<vtkImporter> GetImporter(const std::string& fileName, bool geometry)
  {
    if (!geometry)
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
    importer->SetFileName(fileName);
    if (!importer->CanReadFile())
    {
      return nullptr;
    }
    return importer;
  }

  static void InitializeImporterWithOptions(const options& options, vtkF3DGenericImporter* importer)
  {
    importer->SetPointSize(options.getAsDouble("point-size"));
    importer->SetSurfaceColor(options.getAsDoubleVector("color").data());
    importer->SetOpacity(options.getAsDouble("opacity"));
    importer->SetRoughness(options.getAsDouble("roughness"));
    importer->SetMetallic(options.getAsDouble("metallic"));
    importer->SetLineWidth(options.getAsDouble("line-width"));
    importer->SetTextureBaseColor(options.getAsString("texture-base-color"));
    importer->SetTextureEmissive(options.getAsString("texture-emissive"));
    importer->SetEmissiveFactor(options.getAsDoubleVector("emissive-factor").data());
    importer->SetTextureMaterial(options.getAsString("texture-material"));
    importer->SetTextureNormal(options.getAsString("texture-normal"));
    importer->SetNormalScale(options.getAsDouble("normal-scale"));
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

  std::vector<std::string> FilesList;
  int CurrentFileIndex = 0;
  bool LoadedFile = false;

  const options& Options;
  window_impl& Window;
  interactor_impl* Interactor = nullptr;

  vtkSmartPointer<vtkImporter> Importer;
};

//----------------------------------------------------------------------------
loader_impl::loader_impl(const options& options, window_impl& window)
  : Internals(new loader_impl::internals(options, window))
{
}

//----------------------------------------------------------------------------
loader_impl::~loader_impl() = default;

//----------------------------------------------------------------------------
void loader_impl::addFiles(const std::vector<std::string>& files)
{
  for (auto& file : files)
  {
    this->addFile(file);
  }
}

//----------------------------------------------------------------------------
void loader_impl::addFile(const std::string& path, bool recursive)
{
  if (path.empty())
  {
    return;
  }

  std::string fullPath = vtksys::SystemTools::CollapseFullPath(path);
  if (!vtksys::SystemTools::FileExists(fullPath))
  {
    log::error("File ", fullPath, " does not exist");
    return;
  }

  if (vtksys::SystemTools::FileIsDirectory(fullPath))
  {
    vtksys::Directory dir;
    dir.Load(fullPath);
    std::vector<std::string> sortedFiles;
    sortedFiles.reserve(dir.GetNumberOfFiles());

    // Sorting is necessary as KWSys can provide unsorted files
    for (unsigned long i = 0; i < dir.GetNumberOfFiles(); i++)
    {
      std::string currentFile = dir.GetFile(i);
      if (currentFile != "." && currentFile != "..")
      {
        sortedFiles.push_back(currentFile);
      }
    }
    std::sort(sortedFiles.begin(), sortedFiles.end());

    for (std::string currentFile : sortedFiles)
    {
      std::string newPath = vtksys::SystemTools::JoinPath({ "", fullPath, currentFile });
      if (recursive || !vtksys::SystemTools::FileIsDirectory(newPath))
      {
        this->addFile(newPath, recursive);
      }
    }
  }
  else
  {
    auto it =
      std::find(this->Internals->FilesList.begin(), this->Internals->FilesList.end(), fullPath);

    if (it == this->Internals->FilesList.end())
    {
      this->Internals->FilesList.push_back(fullPath);
    }
  }
}

//----------------------------------------------------------------------------
void loader_impl::getFileInfo(
  LoadFileEnum load, int& nextFileIndex, std::string& filePath, std::string& fileInfo) const
{
  int size = static_cast<int>(this->Internals->FilesList.size());
  if (size > 0)
  {
    int addToIndex = 0;
    bool compute = true;
    switch (load)
    {
      case loader::LoadFileEnum::LOAD_FIRST:
        nextFileIndex = 0;
        compute = false;
        break;
      case loader::LoadFileEnum::LOAD_LAST:
        nextFileIndex = size - 1;
        compute = false;
        break;
      case loader::LoadFileEnum::LOAD_PREVIOUS:
        addToIndex = -1;
        break;
      case loader::LoadFileEnum::LOAD_NEXT:
        addToIndex = 1;
        break;
      case loader::LoadFileEnum::LOAD_CURRENT:
      default:
        break;
    }

    // Compute the correct file index if needed
    if (compute)
    {
      nextFileIndex = (this->Internals->CurrentFileIndex + addToIndex) % size;
      nextFileIndex = nextFileIndex < 0 ? nextFileIndex + size : nextFileIndex;
    }

    filePath = this->Internals->FilesList[nextFileIndex];
    fileInfo = "(" + std::to_string(nextFileIndex + 1) + "/" + std::to_string(size) + ") " +
      vtksys::SystemTools::GetFilenameName(filePath);
  }
  else
  {
    nextFileIndex = -1;
  }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& loader_impl::getFiles() const
{
  return this->Internals->FilesList;
}

//----------------------------------------------------------------------------
void loader_impl::setCurrentFileIndex(int index)
{
  this->Internals->CurrentFileIndex = index;
}

//----------------------------------------------------------------------------
int loader_impl::getCurrentFileIndex() const
{
  return this->Internals->CurrentFileIndex;
}

//----------------------------------------------------------------------------
bool loader_impl::loadFile(loader::LoadFileEnum load)
{
  // Reset loadedFile
  this->Internals->LoadedFile = false;

  // Recover information about the file to load
  std::string filePath, fileInfo;
  int nextFileIndex;
  this->getFileInfo(load, nextFileIndex, filePath, fileInfo);

  if (filePath.empty())
  {
    // No file provided, show a drop zone instead
    log::debug("No file to load provided\n");
    fileInfo += "No file to load provided, please drop one into this window";
    this->Internals->Window.Initialize(false, fileInfo);
    this->Internals->Window.update();
    return this->Internals->LoadedFile;
  }

  // There is a file to load, update CurrentFileIndex
  log::debug("Loading: ", filePath, "\n");
  this->Internals->CurrentFileIndex = nextFileIndex;

  // Recover the importer
  this->Internals->Importer = loader_impl::internals::GetImporter(
    filePath, this->Internals->Options.getAsBool("geometry-only"));
  vtkF3DGenericImporter* genericImporter =
    vtkF3DGenericImporter::SafeDownCast(this->Internals->Importer);
  if (!this->Internals->Importer)
  {
    log::warn(filePath, " is not a file of a supported file format\n");
    fileInfo += " [UNSUPPORTED]";
    this->Internals->Window.Initialize(false, fileInfo);
    this->Internals->Window.update();
    return this->Internals->LoadedFile;
  }

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  loader_impl::internals::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;

  this->Internals->Window.Initialize(genericImporter != nullptr, fileInfo);

  // Initialize importer for rendering
  this->Internals->Importer->SetRenderWindow(this->Internals->Window.GetRenderWindow());

// Importer camera needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7701
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210303)
  this->Internals->Importer->SetCamera(this->Internals->Options.getAsInt("camera-index"));
#else
  // XXX There is no way to recover the init value yet, assume it is -1
  if (this->Internals->Options.getAsInt("camera-index") != -1)
  {
    log::warn("This VTK version does not support specifying the camera index, ignored.");
  }
#endif

  // Manage progress bar
  if (this->Internals->Options.getAsBool("progress") && this->Internals->Interactor)
  {
    loader_impl::internals::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->Importer, this->Internals->Interactor);
  }

  // Initialize genericImporter with options
  if (genericImporter)
  {
    loader_impl::internals::InitializeImporterWithOptions(
      this->Internals->Options, genericImporter);
  }

  // Read the file
  this->Internals->Importer->Update();
  loader_impl::internals::DisplayImporterDescription(this->Internals->Importer);

  // Remove anything progress related if any
  this->Internals->Importer->RemoveObservers(vtkCommand::ProgressEvent);
  progressWidget->Off();

  if (this->Internals->Interactor)
  {
    // Initialize the animation using temporal information from the importer
    this->Internals->Interactor->InitializeAnimation(this->Internals->Importer);
  }

  // Recover generic importer specific actors and mappers to set on the renderer with coloring
  if (genericImporter)
  {
    this->Internals->Window.InitializeRendererWithColoring(genericImporter);
  }

  // Initialize renderer using data read by the importer
  this->Internals->Window.update();

  this->Internals->LoadedFile = true;
  return this->Internals->LoadedFile;
}

//----------------------------------------------------------------------------
void loader_impl::setInteractor(interactor_impl* interactor)
{
  this->Internals->Interactor = interactor;
}
}
