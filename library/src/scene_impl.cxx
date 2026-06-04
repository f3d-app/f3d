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
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkLightCollection.h>
#include <vtkMemoryResourceStream.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkTimerLog.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

// requires https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12411
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251110)
#include <vtkStridedArray.h>
#endif

#include <numeric>
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

  void Load(const std::vector<std::pair<std::string, vtkSmartPointer<vtkImporter>>>& importers)
  {
    for (const auto& importer : importers)
    {
      this->MetaImporter->AddImporter(importer);
    }

    // Initialize the camera on load
    this->Window.InitializeUpDirection();

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
    if (this->Interactor)
    {
      f3d::color_t color = this->Options.ui.loader_progress_color;
      scene_impl::internals::CreateProgressRepresentationAndCallback(
        &callbackData, this->MetaImporter, this->Interactor, color);
    }

    // Update the meta importer, the will only update importers that have not been updated before
    if (!this->MetaImporter->Update())
    {
      this->MetaImporter->RemoveObservers(vtkCommand::ProgressEvent);
      progressWidget->Off();

      this->MetaImporter->Clear();
      this->Window.Initialize();
      throw scene::load_failure_exception("failed to load scene");
    }

    // Remove anything progress related if any
    this->MetaImporter->RemoveObservers(vtkCommand::ProgressEvent);
    progressWidget->Off();

    // Initialize the animation using temporal information from the importer
    this->AnimationManager.UpdateDynamicOptions();
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

  std::vector<std::pair<std::string, vtkSmartPointer<vtkImporter>>> importers;
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
    const std::optional<bool> skipContentCheck = this->Internals->Options.scene.skip_content_check;
    file_availability availability = f3d::file_availability::UNSUPPORTED_EXTENSION;
    // Recover the importer for the provided file path
    const f3d::reader* reader = f3d::factory::instance()->getReader(
      filePath.string(), forceReader, skipContentCheck, availability);
    if (availability == file_availability::SUPPORTED)
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
      std::string errorMessage;
      switch (availability)
      {
        case file_availability::UNSUPPORTED_EXTENSION:
          errorMessage = (filePath.string() +
            " is not a file of a supported 3D scene file format, use force reader to force a "
            "specific "
            "reader");
          break;
        case file_availability::UNSUPPORTED_CONTENT:
          errorMessage = (filePath.string() +
            " contains unsupported content");
          break;
        default:
          errorMessage = (filePath.string() +
            " is not a file of a supported 3D scene file format or contains unsupported content");
          break;
      }
      throw scene::load_failure_exception(errorMessage);
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
    importers.emplace_back(filePath.filename().string(), importer);
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
scene& scene_impl::add(const std::byte* buffer, std::size_t size)
{
  if (buffer == nullptr || size == 0)
  {
    log::debug("Empty buffer or zero size when trying to load a buffer into the scene provided\n");
    return *this;
  }

  // Recover the appropriate reader
  std::optional<std::string> forceReader = this->Internals->Options.scene.force_reader;
  const std::optional<bool> skipContentCheck = this->Internals->Options.scene.skip_content_check;

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 20260128)
  if (!forceReader)
  {
    throw scene::load_failure_exception(
      "No force reader set while trying to load a buffer from memory");
  }
#endif

  const f3d::reader* reader =
    f3d::factory::instance()->getReader(buffer, size, forceReader, skipContentCheck);
  if (reader)
  {
    if (forceReader)
    {
      log::debug("Forcing reader ", (*forceReader), " for stream");
    }
    else
    {
      log::debug("Found a reader for stream:  \"", reader->getName(), "\"");
    }
  }
  else
  {
    if (forceReader)
    {
      throw scene::load_failure_exception(*forceReader + " is not a valid force reader");
    }
    throw scene::load_failure_exception("provided stream is not a file of a supported 3D scene "
                                        "file format, use force reader to force a specific reader");
  }

  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(buffer, size);

  vtkSmartPointer<vtkImporter> importer = reader->createSceneReader(stream);
  if (!importer)
  {
    auto vtkReader = reader->createGeometryReader(stream);

    if (!vtkReader)
    {
      throw scene::load_failure_exception(reader->getName() + " does not support reading streams");
    }

    vtkNew<vtkF3DGenericImporter> genericImporter;
    genericImporter->SetInternalReader(vtkReader);
    importer = genericImporter;
  }

  log::debug("\nLoading stream");
  this->Internals->Load({ { "<stream>", importer } });
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

  vtkSource->SetUpdateFunction(
    [=](double, vtkPolyData* polydata)
    {
      vtkNew<vtkFloatArray> positions;
      positions->SetName("Positions");
      positions->SetNumberOfComponents(3);
      positions->SetNumberOfTuples(mesh.points.size() / 3);
      std::ranges::copy(mesh.points, positions->Begin());

      vtkNew<vtkPoints> points;
      points->SetData(positions);

      polydata->SetPoints(points);

      if (mesh.normals.size() > 0)
      {
        vtkNew<vtkFloatArray> normals;
        normals->SetName("Normals");
        normals->SetNumberOfComponents(3);
        normals->SetNumberOfTuples(mesh.points.size() / 3);
        std::ranges::copy(mesh.normals, normals->Begin());

        polydata->GetPointData()->SetNormals(normals);
      }

      if (mesh.texture_coordinates.size() > 0)
      {
        vtkNew<vtkFloatArray> tcoords;
        tcoords->SetName("TCoords");
        tcoords->SetNumberOfComponents(2);
        tcoords->SetNumberOfTuples(mesh.points.size() / 3);
        std::ranges::copy(mesh.texture_coordinates, tcoords->Begin());

        polydata->GetPointData()->SetTCoords(tcoords);
      }

      vtkNew<vtkIdTypeArray> offsets;
      vtkNew<vtkIdTypeArray> connectivity;

      offsets->SetNumberOfTuples(mesh.face_sides.size() + 1);
      connectivity->SetNumberOfTuples(mesh.face_indices.size());

      // fill offsets
      offsets->SetValue(0, 0);
      std::inclusive_scan(mesh.face_sides.begin(), mesh.face_sides.end(), offsets->Begin() + 1);

      // fill connectivity
      std::ranges::copy(mesh.face_indices, connectivity->Begin());

      vtkNew<vtkCellArray> polys;
      polys->SetData(offsets, connectivity);
      polydata->SetPolys(polys);
    });

  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetInternalReader(vtkSource);

  log::debug("Loading 3D scene from memory");
  this->Internals->Load({ { "<mesh>", importer } });
  return *this;
}

//----------------------------------------------------------------------------
scene& scene_impl::add([[maybe_unused]] std::shared_ptr<mesh_view> mesh)
{
  if (!mesh)
  {
    throw scene::load_failure_exception("Null mesh view provided");
  }

  // requires https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12411
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251110)
  vtkNew<vtkF3DMemoryMesh> vtkSource;

  // Only time range is supported at the moment but we should also add support
  // for time steps and simulated meshes in the future
  auto timeRange = mesh->getTimeRange();
  vtkSource->SetTimeRange(timeRange[0], timeRange[1]);

  vtkSource->SetUpdateFunction(
    [=](double time, vtkPolyData* polydata)
    {
      const auto memoryView = mesh->getMemoryView(time);

      bool firstTime = polydata->GetPoints() == nullptr;

      f3d::log::debug(firstTime ? "Initializing" : "Updating", " mesh_view at time ", time);

      // handle points
      if (memoryView.pointCount == 0)
      {
        throw scene::load_failure_exception("Mesh view must have points");
      }

      if (memoryView.points.data == nullptr)
      {
        throw scene::load_failure_exception("Mesh view points data pointer is null");
      }

      if (memoryView.points.type != mesh_view::data_type::F32 &&
        memoryView.points.type != mesh_view::data_type::F64)
      {
        throw scene::load_failure_exception("Mesh view points must have a data type of F32 or F64");
      }

      if (memoryView.points.components != 3)
      {
        throw scene::load_failure_exception("Mesh view points must have 3 components");
      }

      if (firstTime || memoryView.points.timeDependent)
      {
        vtkNew<vtkPoints> points;

        f3d::mesh_view::dataTypeDispatch(memoryView.points.type,
          [&]<typename DataT>()
          {
            vtkNew<vtkStridedArray<DataT>> positions;
            positions->SetName(
              memoryView.points.name.empty() ? "Positions" : memoryView.points.name.c_str());
            positions->SetNumberOfComponents(3);
            positions->SetNumberOfTuples(memoryView.pointCount);
            positions->ConstructBackend(
              reinterpret_cast<const DataT*>(memoryView.points.data), memoryView.points.stride, 3);

            points->SetData(positions);
          });

        polydata->SetPoints(points);
      }

      // handle normals if provided
      if (memoryView.normals.data != nullptr && (firstTime || memoryView.normals.timeDependent))
      {
        if (memoryView.normals.type != mesh_view::data_type::F32 &&
          memoryView.normals.type != mesh_view::data_type::F64)
        {
          throw scene::load_failure_exception(
            "Mesh view normals must have a data type of F32 or F64");
        }

        if (memoryView.normals.components != 3)
        {
          throw scene::load_failure_exception("Mesh view normals must have 3 components");
        }

        f3d::mesh_view::dataTypeDispatch(memoryView.normals.type,
          [&]<typename DataT>()
          {
            vtkNew<vtkStridedArray<DataT>> normals;
            normals->SetName(
              memoryView.normals.name.empty() ? "Normals" : memoryView.normals.name.c_str());
            normals->SetNumberOfComponents(3);
            normals->SetNumberOfTuples(memoryView.pointCount);
            normals->ConstructBackend(reinterpret_cast<const DataT*>(memoryView.normals.data),
              memoryView.normals.stride, 3);

            polydata->GetPointData()->SetNormals(normals);
          });
      }

      // handle texture coordinates if provided
      if ((firstTime || memoryView.textureCoordinates.timeDependent) &&
        memoryView.textureCoordinates.data != nullptr)
      {
        if (memoryView.textureCoordinates.type != mesh_view::data_type::F32 &&
          memoryView.textureCoordinates.type != mesh_view::data_type::F64)
        {
          throw scene::load_failure_exception(
            "Mesh view texture coordinates must have a data type of F32 or F64");
        }

        if (memoryView.textureCoordinates.components != 2)
        {
          throw scene::load_failure_exception(
            "Mesh view texture coordinates must have 2 components");
        }

        f3d::mesh_view::dataTypeDispatch(memoryView.textureCoordinates.type,
          [&]<typename DataT>()
          {
            vtkNew<vtkStridedArray<DataT>> tcoords;
            tcoords->SetName(memoryView.textureCoordinates.name.empty()
                ? "TCoords"
                : memoryView.textureCoordinates.name.c_str());
            tcoords->SetNumberOfComponents(2);
            tcoords->SetNumberOfTuples(memoryView.pointCount);
            tcoords->ConstructBackend(
              reinterpret_cast<const DataT*>(memoryView.textureCoordinates.data),
              memoryView.textureCoordinates.stride, 2);

            polydata->GetPointData()->SetTCoords(tcoords);
          });
      }

      // handle scalars if provided
      for (const auto& scalar : memoryView.pointScalars)
      {
        if (firstTime || scalar.timeDependent)
        {
          f3d::mesh_view::dataTypeDispatch(scalar.type,
            [&]<typename DataT>()
            {
              vtkNew<vtkStridedArray<DataT>> scalars;
              scalars->SetName(scalar.name.c_str());
              scalars->SetNumberOfComponents(static_cast<int>(scalar.components));
              scalars->SetNumberOfTuples(memoryView.pointCount);
              scalars->ConstructBackend(reinterpret_cast<const DataT*>(scalar.data), scalar.stride,
                static_cast<int>(scalar.components));

              polydata->GetPointData()->AddArray(scalars);
            });
        }
      }

      for (const auto& scalar : memoryView.cellScalars)
      {
        if (firstTime || scalar.timeDependent)
        {
          f3d::mesh_view::dataTypeDispatch(scalar.type,
            [&]<typename DataT>()
            {
              vtkNew<vtkStridedArray<DataT>> scalars;
              scalars->SetName(scalar.name.c_str());
              scalars->SetNumberOfComponents(static_cast<int>(scalar.components));
              scalars->SetNumberOfTuples(memoryView.vertices.offsetCount +
                memoryView.lines.offsetCount + memoryView.polygons.offsetCount - 3);
              scalars->ConstructBackend(reinterpret_cast<const DataT*>(scalar.data), scalar.stride,
                static_cast<int>(scalar.components));

              polydata->GetCellData()->AddArray(scalars);
            });
        }
      }

      auto handleCells =
        [](const f3d::mesh_view::cell_array_t& cells) -> vtkSmartPointer<vtkCellArray>
      {
        if (cells.offsetCount <= 0)
        {
          throw scene::load_failure_exception(
            "Mesh view cell offsets count must be greater than 0");
        }

        if (cells.offsetCount == 1) // means there is no cell
        {
          return nullptr;
        }

        if (cells.offsets.data == nullptr)
        {
          throw scene::load_failure_exception("Mesh view cell offsets pointer is null");
        }

        if (cells.indices.data == nullptr)
        {
          throw scene::load_failure_exception("Mesh view cell indices pointer is null");
        }

        if (cells.offsets.type != mesh_view::data_type::I32 &&
          cells.offsets.type != mesh_view::data_type::U32 &&
          cells.offsets.type != mesh_view::data_type::I64 &&
          cells.offsets.type != mesh_view::data_type::U64)
        {
          throw scene::load_failure_exception(
            "Mesh view cell offsets must have a data type of I32, U32, I64, or U64");
        }

        if (cells.indices.type != mesh_view::data_type::I32 &&
          cells.indices.type != mesh_view::data_type::U32 &&
          cells.indices.type != mesh_view::data_type::I64 &&
          cells.indices.type != mesh_view::data_type::U64)
        {
          throw scene::load_failure_exception(
            "Mesh view cell indices must have a data type of I32, U32, I64, or U64");
        }

        if (cells.indices.type != cells.offsets.type)
        {
          throw scene::load_failure_exception(
            "Mesh view cell offsets and cell indices must have the same data type");
        }

        return f3d::mesh_view::dataTypeDispatch(cells.offsets.type,
          [&]<typename DataT>() -> vtkSmartPointer<vtkCellArray>
          {
            if constexpr (std::is_integral_v<DataT>) // makes no sense for F32 or F64
            {
              // if the user provided unsigned data, we need to use the corresponding signed type
              // for VTK
              using IndexingType = std::make_signed_t<DataT>;

              vtkNew<vtkCellArray> cellArray;

              vtkNew<vtkStridedArray<IndexingType>> faceOffsets;
              faceOffsets->SetName(
                cells.offsets.name.empty() ? "FaceOffsets" : cells.offsets.name.c_str());
              faceOffsets->SetNumberOfTuples(cells.offsetCount);
              faceOffsets->ConstructBackend(
                reinterpret_cast<const IndexingType*>(cells.offsets.data), cells.offsets.stride);

              vtkNew<vtkStridedArray<IndexingType>> faceIndices;
              faceIndices->SetName(
                cells.indices.name.empty() ? "FaceIndices" : cells.indices.name.c_str());
              faceIndices->SetNumberOfTuples(cells.indexCount);
              faceIndices->ConstructBackend(
                reinterpret_cast<const IndexingType*>(cells.indices.data), cells.indices.stride);

              cellArray->SetData(faceOffsets, faceIndices);
              return cellArray;
            }
            return nullptr;
          });
      };

      if (memoryView.vertices.indices.timeDependent || memoryView.vertices.offsets.timeDependent ||
        firstTime)
      {
        polydata->SetVerts(handleCells(memoryView.vertices));
      }

      if (memoryView.lines.indices.timeDependent || memoryView.lines.offsets.timeDependent ||
        firstTime)
      {
        polydata->SetLines(handleCells(memoryView.lines));
      }

      if (memoryView.polygons.indices.timeDependent || memoryView.polygons.offsets.timeDependent ||
        firstTime)
      {
        polydata->SetPolys(handleCells(memoryView.polygons));
      }
    });

  try
  {
    vtkSource->Update();
  }
  catch (const load_failure_exception& e)
  {
    throw load_failure_exception(std::string("Failed to load mesh from memory: ") + e.what());
  }

  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetInternalReader(vtkSource);

  std::string name = mesh->getName();

  log::debug("Loading 3D scene from memory");
  this->Internals->Load({ { name.empty() ? "<mesh_view>" : name, importer } });
  return *this;
#else
  throw scene::load_failure_exception(
    "Loading a mesh from memory using add(std::shared_ptr<mesh>) requires VTK >= 9.6");
#endif
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
f3d::file_availability scene_impl::supports(const fs::path& filePath)
{
  f3d::file_availability availability =
    f3d::file_availability::UNSUPPORTED_EXTENSION;
  f3d::factory::instance()->getReader(filePath.string(),
    this->Internals->Options.scene.force_reader, this->Internals->Options.scene.skip_content_check,
    availability) != nullptr;
  return availability;
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
std::vector<double> scene_impl::getAnimationKeyFrames()
{
  return this->Internals->AnimationManager.GetKeyFrames();
}

//----------------------------------------------------------------------------
unsigned int scene_impl::availableAnimations() const
{
  return this->Internals->AnimationManager.GetNumberOfAvailableAnimations();
}

//----------------------------------------------------------------------------
std::string scene_impl::getAnimationName(int index)
{
  return this->Internals->AnimationManager.GetAnimationName(index);
}

//----------------------------------------------------------------------------
std::vector<std::string> scene_impl::getAnimationNames()
{
  return this->Internals->AnimationManager.GetAnimationNames();
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
