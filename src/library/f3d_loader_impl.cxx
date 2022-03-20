#include "f3d_loader_impl.h"

#include "F3DReaderFactory.h"
#include "F3DReaderInstantiator.h"
#include "f3d_interactor_impl.h"
#include "f3d_log.h"
#include "f3d_options.h"
#include "f3d_window.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DObjectFactory.h"

#include <vtkCallbackCommand.h>
#include <vtkLogger.h>
#include <vtkNew.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <set>

namespace
{
typedef struct ProgressDataStruct
{
  vtkTimerLog* timer;
  vtkProgressBarWidget* widget;
} ProgressDataStruct;

vtkSmartPointer<vtkImporter> GetImporter(const std::string& fileName, bool geometry)
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
  importer->SetFileName(fileName.c_str());
  if (!importer->CanReadFile())
  {
    return nullptr;
  }
  return importer;
}

void InitializeImporterWithOptions(const f3d::options& options, vtkF3DGenericImporter* importer)
{
  importer->SetPointSize(options.getAsDouble("point-size"));
  importer->SetScalarArray(options.getAsString("scalars"));
  importer->SetUseCellScalars(options.getAsBool("cells"));
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

void CreateProgressRepresentationAndCallback(
  ProgressDataStruct* data, vtkImporter* importer, f3d::interactor_impl* interactor)
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

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201027)
  progressRep->DrawFrameOff();
  progressRep->SetPadding(0.0, 0.0);
#endif
  data->timer->StartTimer();
}

void DisplayImporterDescription(vtkImporter* importer)
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210228)
  vtkIdType availCameras = importer->GetNumberOfCameras();
  if (availCameras <= 0)
  {
    f3d::log::info("No camera available in this file");
  }
  else
  {
    f3d::log::info("Camera(s) available in this file are:");
  }
  for (int i = 0; i < availCameras; i++)
  {
    f3d::log::info(i, ": ", importer->GetCameraName(i));
  }
  f3d::log::info("\n");
#endif
  f3d::log::info(importer->GetOutputsDescription());
  f3d::log::info("\n");
}
}

namespace f3d
{
class loader_impl::F3DInternals
{
public:
  F3DInternals(const options& options, window& window)
    : Options(options)
    , Window(window)
  {
  }
  std::vector<std::string> FilesList;
  int CurrentFileIndex = 0;
  bool LoadedFile = false;
  const options& Options;
  window& Window;
  interactor_impl* Interactor = nullptr;
  vtkSmartPointer<vtkImporter> Importer;
  F3DReaderInstantiator ReaderInstantiator;
};

//----------------------------------------------------------------------------
loader_impl::loader_impl(const options& options, window& window)
  : Internals(new loader_impl::F3DInternals(options, window))
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // Disable vtkLogger in case VTK was built with log support
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200701)
  vtkLogger::SetInternalVerbosityLevel(vtkLogger::VERBOSITY_OFF);
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  // Make sure to initialize the output window
  // after the object factory and before the first usage.
  f3d::log::setQuiet(false);
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
    f3d::log::error("File ", fullPath, " does not exist");
    return;
  }

  if (vtksys::SystemTools::FileIsDirectory(fullPath))
  {
    if (recursive)
    {
      vtksys::Directory dir;
      dir.Load(fullPath);
      std::set<std::string> sortedFiles;

      // Sorting is necessary as KWSys can provide unsorted files
      for (unsigned long i = 0; i < dir.GetNumberOfFiles(); i++)
      {
        std::string currentFile = dir.GetFile(i);
        if (currentFile != "." && currentFile != "..")
        {
          sortedFiles.insert(currentFile);
        }
      }
      for (std::string currentFile : sortedFiles)
      {
        this->addFile(vtksys::SystemTools::JoinPath({ "", fullPath, currentFile }), false);
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
    switch (load)
    {
      case loader::LoadFileEnum::LOAD_FIRST:
        nextFileIndex = 0;
        break;
      case loader::LoadFileEnum::LOAD_LAST:
        nextFileIndex = size - 1;
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

    // Compute the correct file index
    nextFileIndex = (this->Internals->CurrentFileIndex + addToIndex) % size;
    nextFileIndex = nextFileIndex < 0 ? nextFileIndex + size : nextFileIndex;

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
std::vector<std::string> loader_impl::getFiles()
{
  return this->Internals->FilesList;
}

//----------------------------------------------------------------------------
void loader_impl::setCurrentFileIndex(int index)
{
  this->Internals->CurrentFileIndex = index;
}

//----------------------------------------------------------------------------
int loader_impl::getCurrentFileIndex()
{
  return this->Internals->CurrentFileIndex;
}

//----------------------------------------------------------------------------
bool loader_impl::loadFile(loader::LoadFileEnum load)
{
  // Reset loadedFile
  this->Internals->LoadedFile = false;

  f3d::log::setQuiet(this->Internals->Options.getAsBool("quiet"));

  // Recover information about the file to load
  std::string filePath, fileInfo;
  this->getFileInfo(load, this->Internals->CurrentFileIndex, filePath, fileInfo);
  bool verbose = this->Internals->Options.getAsBool("verbose");
  if (verbose)
  {
    if (filePath.empty())
    {
      f3d::log::info("No file to load provided\n");
    }
    else
    {
      f3d::log::info("Loading: ", filePath, "\n");
    }
  }

  // No file provided, show a drop zone instead
  if (filePath.empty())
  {
    fileInfo += "No file to load provided, please drop one into this window";
    this->Internals->Window.Initialize(false, fileInfo);
    this->Internals->Window.update();
    return this->Internals->LoadedFile;
  }

  // Recover the importer
  this->Internals->Importer =
    ::GetImporter(filePath, this->Internals->Options.getAsBool("geometry-only"));
  vtkF3DGenericImporter* genericImporter =
    vtkF3DGenericImporter::SafeDownCast(this->Internals->Importer);
  if (!this->Internals->Importer)
  {
    f3d::log::warn(filePath, " is not a file of a supported file format\n");
    fileInfo += " [UNSUPPORTED]";
    this->Internals->Window.Initialize(false, fileInfo);
    this->Internals->Window.update();
    return this->Internals->LoadedFile;
  }

  vtkNew<vtkProgressBarWidget> progressWidget;
  vtkNew<vtkTimerLog> timer;
  ::ProgressDataStruct callbackData;
  callbackData.timer = timer;
  callbackData.widget = progressWidget;

  this->Internals->Window.Initialize(genericImporter != nullptr, fileInfo);

  // Initialize importer for rendering
  this->Internals->Importer->SetRenderWindow(this->Internals->Window.GetRenderWindow());
#if VTK_VERSION_NUMBER > VTK_VERSION_CHECK(9, 0, 20210228)
  this->Internals->Importer->SetCamera(this->Internals->Options.getAsInt("camera-index"));
#endif

  // Manage progress bar
  if (this->Internals->Options.getAsBool("progress") && this->Internals->Interactor)
  {
    ::CreateProgressRepresentationAndCallback(
      &callbackData, this->Internals->Importer, this->Internals->Interactor);
  }

  // Initialize genericImporter with options
  if (genericImporter)
  {
    ::InitializeImporterWithOptions(this->Internals->Options, genericImporter);
  }

  // Read the file
  this->Internals->Importer->Update();
  if (verbose)
  {
    ::DisplayImporterDescription(this->Internals->Importer);
  }

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
