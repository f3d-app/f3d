#include "vtkF3DMetaImporter.h"

#include "F3DLog.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DImporter.h"

#include <vtkActorCollection.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkVersion.h>

#include <cassert>
#include <iostream>
#include <numeric>
#include <vector>

#include "vtkF3DGLTFImporter.h"

//----------------------------------------------------------------------------
struct vtkF3DMetaImporter::Internals
{
  // Actors related vectors
  std::vector<vtkF3DMetaImporter::ColoringStruct> ColoringActorsAndMappers;
  std::vector<vtkF3DMetaImporter::PointSpritesStruct> PointSpritesActorsAndMappers;
  std::vector<vtkF3DMetaImporter::VolumeStruct> VolumePropsAndMappers;

  struct ImporterPair
  {
    vtkSmartPointer<vtkImporter> Importer;
    bool Updated = false;
  };
  std::vector<ImporterPair> Importers;
  std::optional<vtkIdType> CameraIndex;
  vtkBoundingBox GeometryBoundingBox;
  vtkTimeStamp ColoringInfoTime;
  vtkTimeStamp UpdateTime;

  F3DColoringInfoHandler ColoringInfoHandler;

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20240707)
  std::map<vtkImporter*, vtkSmartPointer<vtkActorCollection>> ActorsForImporterMap;
#endif
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DMetaImporter);

//----------------------------------------------------------------------------
vtkF3DMetaImporter::vtkF3DMetaImporter()
  : Pimpl(new Internals())
{
}

//----------------------------------------------------------------------------
vtkF3DMetaImporter::~vtkF3DMetaImporter()
{
  // XXX by doing this we ensure ~vtkImporter does not delete it
  // As we have our own way of handling renderer lifetime
  this->Renderer = nullptr;
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::Clear()
{
  this->Pimpl->Importers.clear();
  this->Pimpl->GeometryBoundingBox.Reset();
  this->ActorCollection->RemoveAllItems();
  this->Pimpl->ColoringActorsAndMappers.clear();
  this->Pimpl->PointSpritesActorsAndMappers.clear();
  this->Pimpl->VolumePropsAndMappers.clear();
  this->Pimpl->ColoringInfoHandler.ClearColoringInfo();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::AddImporter(const vtkSmartPointer<vtkImporter>& importer)
{
  this->Pimpl->Importers.emplace_back(
    vtkF3DMetaImporter::Internals::ImporterPair{ importer, false });
  this->Modified();

  // Add a progress event observer
  vtkNew<vtkCallbackCommand> progressCallback;
  progressCallback->SetClientData(this);
  progressCallback->SetCallback(
    [](vtkObject* const caller, unsigned long, void* clientData, void* callData)
    {
      vtkF3DMetaImporter* self = static_cast<vtkF3DMetaImporter*>(clientData);
      double progress = *static_cast<double*>(callData);
      double actualProgress = 0.0;
      for (size_t i = 0; i < self->Pimpl->Importers.size(); i++)
      {
        if (self->Pimpl->Importers[i].Importer == caller)
        {
          // XXX: This does not consider that some importer may already have been updated
          // or that some importers may take much longer than other.
          actualProgress = (i + progress) / self->Pimpl->Importers.size();
        }
      }
      self->InvokeEvent(vtkCommand::ProgressEvent, &actualProgress);
    });
  importer->AddObserver(vtkCommand::ProgressEvent, progressCallback);
}

//----------------------------------------------------------------------------
const vtkBoundingBox& vtkF3DMetaImporter::GetGeometryBoundingBox()
{
  return this->Pimpl->GeometryBoundingBox;
}

//----------------------------------------------------------------------------
const std::vector<vtkF3DMetaImporter::ColoringStruct>&
vtkF3DMetaImporter::GetColoringActorsAndMappers()
{
  return this->Pimpl->ColoringActorsAndMappers;
}

//----------------------------------------------------------------------------
const std::vector<vtkF3DMetaImporter::PointSpritesStruct>&
vtkF3DMetaImporter::GetPointSpritesActorsAndMappers()
{
  return this->Pimpl->PointSpritesActorsAndMappers;
}

//----------------------------------------------------------------------------
const std::vector<vtkF3DMetaImporter::VolumeStruct>& vtkF3DMetaImporter::GetVolumePropsAndMappers()
{
  return this->Pimpl->VolumePropsAndMappers;
}

//----------------------------------------------------------------------------
bool vtkF3DMetaImporter::Update()
{
  assert(this->RenderWindow);
  this->Renderer = this->RenderWindow->GetRenderers()->GetFirstRenderer();
  assert(this->Renderer);

  vtkIdType localCameraIndex = -1;

  this->Pimpl->UpdateTime.Modified();

  if (this->Pimpl->CameraIndex.has_value())
  {
    if (this->Pimpl->CameraIndex < 0)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Invalid camera index: " + std::to_string(this->Pimpl->CameraIndex.value()) +
          ". Camera may be incorrect.");
    }
    localCameraIndex = this->Pimpl->CameraIndex.value();
  }

  for (auto& importerPair : this->Pimpl->Importers)
  {
    vtkImporter* importer = importerPair.Importer;

    // Importer has already been updated
    if (importerPair.Updated)
    {
      localCameraIndex -= importer->GetNumberOfCameras();
      continue;
    }

    importer->SetRenderWindow(this->RenderWindow);

    // This is required to avoid updating two times
    // but may cause a warning in VTK
    if (localCameraIndex >= 0)
    {
      importer->SetCamera(localCameraIndex);
    }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    if (!importer->Update())
    {
      return false;
    }
#else
    vtkSmartPointer<vtkActorCollection> actorCollection =
      vtkSmartPointer<vtkActorCollection>::New();

    vtkNew<vtkActorCollection> previousActorCollection;
    vtkActorCollection* currentCollection = this->Renderer->GetActors();
    vtkCollectionSimpleIterator tmpIt;
    currentCollection->InitTraversal(tmpIt);
    while (auto* actor = currentCollection->GetNextActor(tmpIt))
    {
      previousActorCollection->AddItem(actor);
    }

    importer->Update();

    currentCollection = this->Renderer->GetActors();
    currentCollection->InitTraversal(tmpIt);

    vtkCollectionSimpleIterator tmpIt2;
    previousActorCollection->InitTraversal(tmpIt2);
    while (auto* actor = currentCollection->GetNextActor(tmpIt))
    {
      bool found = false;
      while (auto* previousActor = previousActorCollection->GetNextActor(tmpIt2))
      {
        // This is a N^2 loop
        if (previousActor == actor)
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        actorCollection->AddItem(actor);
      }
    }

    // Store the actor collection for further use
    this->Pimpl->ActorsForImporterMap[importer] = actorCollection;
#endif

    localCameraIndex -= importer->GetNumberOfCameras();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    vtkActorCollection* actorCollection = importer->GetImportedActors();
#endif

    // Recover generic importer if any (for indexed access to points/image)
    vtkF3DGenericImporter* genericImporter = vtkF3DGenericImporter::SafeDownCast(importer);
    vtkIdType actorIndex = 0;

    vtkCollectionSimpleIterator ait;
    actorCollection->InitTraversal(ait);
    while (vtkActor* actor = actorCollection->GetNextActor(ait))
    {
      // Add to the actor collection
      this->ActorCollection->AddItem(actor);

      vtkPolyDataMapper* pdMapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
      vtkPolyData* surface = pdMapper->GetInput();

      // convert to PBR materials if needed
      if (!genericImporter && actor->GetProperty()->GetInterpolation() != VTK_PBR)
      {
        actor->GetProperty()->SetInterpolationToPBR();
        actor->GetProperty()->LightingOn();

        // Convert to linear space
        auto toLinear = [](double c) { return std::pow(c, 2.2); };
        double diffuseColor[3];
        actor->GetProperty()->GetColor(diffuseColor);
        actor->GetProperty()->SetColor(
          toLinear(diffuseColor[0]), toLinear(diffuseColor[1]), toLinear(diffuseColor[2]));

        // restore diffuse/specular to 1 and ambient to 0
        actor->GetProperty()->SetSpecular(1.0);
        actor->GetProperty()->SetDiffuse(1.0);
        actor->GetProperty()->SetAmbient(0.0);

        // texture diffuse is now base color
        vtkSmartPointer<vtkTexture> diffuseTex = actor->GetTexture();
        if (!diffuseTex)
        {
          diffuseTex = actor->GetProperty()->GetTexture("diffuseTex");
        }
        if (diffuseTex)
        {
          actor->SetTexture(nullptr);
          diffuseTex->UseSRGBColorSpaceOn();

          actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
          actor->GetProperty()->SetBaseColorTexture(diffuseTex);
        }
      }

      // Increase bounding box size if needed
      double bounds[6];
      surface->GetBounds(bounds);
      this->Pimpl->GeometryBoundingBox.AddBounds(bounds);

      // Create and configure coloring actors
      this->Pimpl->ColoringActorsAndMappers.emplace_back(vtkF3DMetaImporter::ColoringStruct(actor));
      vtkF3DMetaImporter::ColoringStruct& cs = this->Pimpl->ColoringActorsAndMappers.back();
      cs.Mapper->SetInputData(surface);
      this->Renderer->AddActor(cs.Actor);
      cs.Actor->VisibilityOff();

      // Create and configure point sprites actors
      this->Pimpl->PointSpritesActorsAndMappers.emplace_back(
        vtkF3DMetaImporter::PointSpritesStruct(actor, importer));
      vtkF3DMetaImporter::PointSpritesStruct& pss =
        this->Pimpl->PointSpritesActorsAndMappers.back();

      vtkPolyData* points = surface;
      if (genericImporter)
      {
        // Use indexed accessor for composite support
        points = genericImporter->GetImportedPoints(actorIndex);
      }
      pss.Mapper->SetInputData(points);
      this->Renderer->AddActor(pss.Actor);
      pss.Actor->VisibilityOff();

      // Create and configure volume props
      if (genericImporter)
      {
        vtkImageData* image = genericImporter->GetImportedImage(actorIndex);
        if (image)
        {
          // XXX: Note that creating this struct takes some time
          this->Pimpl->VolumePropsAndMappers.emplace_back(vtkF3DMetaImporter::VolumeStruct());
          vtkF3DMetaImporter::VolumeStruct& vs = this->Pimpl->VolumePropsAndMappers.back();
          vs.Mapper->SetInputData(image);
          this->Renderer->AddVolume(vs.Prop);
          vs.Prop->VisibilityOff();
        }
      }

      actorIndex++;
    }

    importerPair.Updated = true;
  }

  if (localCameraIndex > 0)
  {
    // Here we know that CameraIndex has a value
    F3DLog::Print(F3DLog::Severity::Warning,
      "Camera index " + std::to_string(this->Pimpl->CameraIndex.value()) +
        " is higher than the number of available camera in the files. Camera may be incorrect.");
  }

  // XXX: UpdateStatus is not set, but libf3d does not use it
  return true;
}

//----------------------------------------------------------------------------
std::string vtkF3DMetaImporter::GetOutputsDescription()
{
  std::string description =
    "Number of files: " + std::to_string(this->Pimpl->Importers.size()) + "\n";
  description +=
    "Number of actors: " + std::to_string(this->ActorCollection->GetNumberOfItems()) + "\n";
  description += std::accumulate(this->Pimpl->Importers.begin(), this->Pimpl->Importers.end(),
    std::string(), [](const std::string& a, const auto& importerPair)
    { return a + "----------\n" + importerPair.Importer->GetOutputsDescription(); });
  return description;
}

//----------------------------------------------------------------------------
vtkF3DImporter::AnimationSupportLevel vtkF3DMetaImporter::GetAnimationSupportLevel()
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 4, 20250507)
  vtkF3DImporter::AnimationSupportLevel levelAccum = vtkF3DImporter::AnimationSupportLevel::MULTI;
#else
  vtkImporter::AnimationSupportLevel levelAccum = vtkImporter::AnimationSupportLevel::NONE;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    AnimationSupportLevel level = importerPair.Importer->GetAnimationSupportLevel();
    switch (level)
    {
      case vtkImporter::AnimationSupportLevel::NONE:
        // Nothing to do, levelAccum is not impacted
        break;
      case vtkImporter::AnimationSupportLevel::UNIQUE:
        switch (levelAccum)
        {
          case vtkImporter::AnimationSupportLevel::NONE:
            // UNIQUE + NONE = UNIQUE
            levelAccum = vtkImporter::AnimationSupportLevel::UNIQUE;
            break;
          case vtkImporter::AnimationSupportLevel::UNIQUE:
            // UNIQUE + UNIQUE = MULTI
            levelAccum = vtkImporter::AnimationSupportLevel::MULTI;
            break;
          default:
            // Other values have no impact on levelAccum
            break;
        }
        break;
      case vtkImporter::AnimationSupportLevel::SINGLE:
        // SINGLE + Any = SINGLE
        levelAccum = vtkImporter::AnimationSupportLevel::SINGLE;
        break;
      case vtkImporter::AnimationSupportLevel::MULTI:
        // MULTI + SINGLE = SINGLE
        // MULTI + Anything else = MULTI
        levelAccum = levelAccum == vtkImporter::AnimationSupportLevel::SINGLE
          ? vtkImporter::AnimationSupportLevel::SINGLE
          : vtkImporter::AnimationSupportLevel::MULTI;
        break;
    }
  }
#endif
  return levelAccum;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DMetaImporter::GetNumberOfAnimations()
{
  // Importer->GetNumberOfAnimations() can be -1 if animation support is not implemented in the
  // importer
  return std::accumulate(this->Pimpl->Importers.begin(), this->Pimpl->Importers.end(), 0,
    [](vtkIdType a, const auto& importerPair)
    {
      vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
      a += nAnim >= 0 ? nAnim : 0;
      return a;
    });
}

//----------------------------------------------------------------------------
std::string vtkF3DMetaImporter::GetAnimationName(vtkIdType animationIndex)
{
  // Importer->GetNumberOfAnimations() can be -1 if animation support is not implemented in the
  // importer
  vtkIdType localAnimationIndex = animationIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      std::string name = importerPair.Importer->GetAnimationName(localAnimationIndex);
      if (name.empty())
      {
        name = "unnamed_" + std::to_string(animationIndex);
      }
      return name;
    }
    else
    {
      localAnimationIndex -= nAnim;
    }
  }
  return "";
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::EnableAnimation(vtkIdType animationIndex)
{
  vtkIdType localAnimationIndex = animationIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      importerPair.Importer->EnableAnimation(localAnimationIndex);
      return;
    }
    else
    {
      localAnimationIndex -= nAnim;
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::DisableAnimation(vtkIdType animationIndex)
{
  vtkIdType localAnimationIndex = animationIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      importerPair.Importer->DisableAnimation(localAnimationIndex);
      return;
    }
    else
    {
      localAnimationIndex -= nAnim;
    }
  }
}

//----------------------------------------------------------------------------
bool vtkF3DMetaImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  vtkIdType localAnimationIndex = animationIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      return importerPair.Importer->IsAnimationEnabled(localAnimationIndex);
    }
    else
    {
      localAnimationIndex -= nAnim;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DMetaImporter::GetNumberOfCameras()
{
  return std::accumulate(this->Pimpl->Importers.begin(), this->Pimpl->Importers.end(), 0,
    [](vtkIdType a, const auto& importerPair)
    { return a + importerPair.Importer->GetNumberOfCameras(); });
}

//----------------------------------------------------------------------------
std::string vtkF3DMetaImporter::GetCameraName(vtkIdType camIndex)
{
  vtkIdType localCameraIndex = camIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nCam = importerPair.Importer->GetNumberOfCameras();
    if (localCameraIndex < nCam)
    {
      std::string name = importerPair.Importer->GetCameraName(localCameraIndex);
      if (name.empty())
      {
        name = "unnamed_" + std::to_string(camIndex);
      }
      return name;
    }
    else
    {
      localCameraIndex -= nCam;
    }
  }
  return "";
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::SetCameraIndex(std::optional<vtkIdType> camIndex)
{
  this->Pimpl->CameraIndex = camIndex;
}

//----------------------------------------------------------------------------
bool vtkF3DMetaImporter::GetTemporalInformation(
  vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps, vtkDoubleArray* timeSteps)
{
  vtkIdType localAnimationIndex = animationIndex;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerPair.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 20251210)
      vtkF3DImporter* f3dImporter = vtkF3DImporter::SafeDownCast(importerPair.Importer);
      if (f3dImporter)
      {
        return f3dImporter->GetTemporalInformation(
          localAnimationIndex, timeRange, nbTimeSteps, timeSteps);
      }
      else
      {
        return importerPair.Importer->GetTemporalInformation(
          localAnimationIndex, 0, nbTimeSteps, timeRange, timeSteps);
      }
#else
      return importerPair.Importer->GetTemporalInformation(
        localAnimationIndex, timeRange, nbTimeSteps, timeSteps);
#endif
    }
    else
    {
      localAnimationIndex -= nAnim;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkF3DMetaImporter::UpdateAtTimeValue(double timeValue)
{
  bool ret = true;
  for (const auto& importerPair : this->Pimpl->Importers)
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
    ret = ret && importerPair.Importer->UpdateAtTimeValue(timeValue);
#else
    importerPair.Importer->UpdateTimeStep(timeValue);
#endif
  }

  // Update coloring and point sprites
  for (auto& cs : this->Pimpl->ColoringActorsAndMappers)
  {
    cs.Mapper->SetInputData(
      vtkPolyDataMapper::SafeDownCast(cs.OriginalActor->GetMapper())->GetInput());

    bool visi = cs.Actor->GetVisibility();
    cs.Actor->vtkProp3D::ShallowCopy(cs.OriginalActor);
    cs.Actor->SetVisibility(visi);
  }
  for (auto& pss : this->Pimpl->PointSpritesActorsAndMappers)
  {
    if (!vtkF3DGenericImporter::SafeDownCast(pss.Importer))
    {
      pss.Mapper->SetInputData(
        vtkPolyDataMapper::SafeDownCast(pss.OriginalActor->GetMapper())->GetInput());
      bool visi = pss.Actor->GetVisibility();
      pss.Actor->vtkProp3D::ShallowCopy(pss.OriginalActor);
      pss.Actor->SetVisibility(visi);
    }
  }

  this->Pimpl->UpdateTime.Modified();
  return ret;
}

//----------------------------------------------------------------------------
void vtkF3DMetaImporter::UpdateInfoForColoring()
{
  if (this->Pimpl->UpdateTime.GetMTime() > this->Pimpl->ColoringInfoTime.GetMTime())
  {
    for (const auto& importerPair : this->Pimpl->Importers)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
      vtkActorCollection* actorCollection = importerPair.Importer->GetImportedActors();
#else
      vtkActorCollection* actorCollection =
        this->Pimpl->ActorsForImporterMap.at(importerPair.Importer).Get();
#endif

      // Recover generic importer if any (for indexed access to points/image)
      vtkF3DGenericImporter* genericImporter =
        vtkF3DGenericImporter::SafeDownCast(importerPair.Importer);
      vtkIdType actorIndex = 0;

      vtkCollectionSimpleIterator ait;
      actorCollection->InitTraversal(ait);
      while (auto* actor = actorCollection->GetNextActor(ait))
      {
        vtkPolyDataMapper* pdMapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        assert(pdMapper);

        // Update coloring vectors, with a dedicated logic for generic importer
        vtkDataSet* datasetForColoring = pdMapper->GetInput();
        if (genericImporter)
        {
          // Use indexed accessor for composite support
          if (genericImporter->GetImportedImage(actorIndex))
          {
            datasetForColoring = genericImporter->GetImportedImage(actorIndex);
          }
          else if (genericImporter->GetImportedPoints(actorIndex))
          {
            datasetForColoring = genericImporter->GetImportedPoints(actorIndex);
          }
        }
        this->Pimpl->ColoringInfoHandler.UpdateColoringInfo(datasetForColoring, false);
        this->Pimpl->ColoringInfoHandler.UpdateColoringInfo(datasetForColoring, true);

        actorIndex++;
      }
    }
    this->Pimpl->ColoringInfoTime.Modified();
  }
}

//----------------------------------------------------------------------------
std::string vtkF3DMetaImporter::GetMetaDataDescription() const
{
  std::string description;
  if (this->Pimpl->Importers.size() > 1)
  {
    description += "Number of files: ";
    description += std::to_string(this->Pimpl->Importers.size());
    description += "\n";
  }

  description += "Number of actors: ";
  description += std::to_string(this->ActorCollection->GetNumberOfItems());
  description += "\n";

  vtkIdType nPoints = 0;
  vtkIdType nCells = 0;
  vtkCollectionSimpleIterator ait;
  this->ActorCollection->InitTraversal(ait);
  while (auto* actor = this->ActorCollection->GetNextActor(ait))
  {
    vtkPolyData* surface = vtkPolyDataMapper::SafeDownCast(actor->GetMapper())->GetInput();
    nPoints += surface->GetNumberOfPoints();
    nCells += surface->GetNumberOfCells();
  }

  description += "Number of points: ";
  description += std::to_string(nPoints);
  description += "\n";
  description += "Number of cells: ";
  description += std::to_string(nCells);
  return description;
}

//----------------------------------------------------------------------------
F3DColoringInfoHandler& vtkF3DMetaImporter::GetColoringInfoHandler()
{
  this->UpdateInfoForColoring();
  return this->Pimpl->ColoringInfoHandler;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkF3DMetaImporter::GetUpdateMTime()
{
  return this->Pimpl->UpdateTime.GetMTime();
}

//----------------------------------------------------------------------------
vtkDataAssembly* vtkF3DMetaImporter::GetSceneHierarchy()
{
    for (const auto& importerPair : this->Pimpl->Importers)
    {
        vtkF3DGLTFImporter* gltfImporter = vtkF3DGLTFImporter::SafeDownCast(importerPair.Importer);
        if (gltfImporter)
        {
            // Returns the GLTF scene hierarchy
            return gltfImporter->GetSceneHierarchy();
        }
    }
    // No GLTF importer found
    return nullptr;
}

// std::vector<NodeInfo> vtkF3DMetaImporter::GetActorHierarchy()
// {
//     std::vector<NodeInfo> nodes;

//     for (const auto& importerPair : this->Pimpl->Importers)
//     {
//         vtkF3DGLTFImporter* gltfImporter = vtkF3DGLTFImporter::SafeDownCast(importerPair.Importer);
//         if (gltfImporter)
//         {
//             vtkRenderer* renderer = gltfImporter->GetRenderer();
//             if (!renderer) continue;

//             vtkPropCollection* props = renderer->GetViewProps();
//             props->InitTraversal();
//             vtkProp* prop = nullptr;

//             while ((prop = props->GetNextProp()))
//             {
//                 if (vtkActor* actor = vtkActor::SafeDownCast(prop))
//                 {
//                     NodeInfo node;
//                     node.actor = actor;
//                     node.name = actor->GetClassName(); // or use gltfImporter-specific node name if available
//                     nodes.push_back(node);
//                 }
//             }
//         }
//     }

//     return nodes;
// }

std::vector<NodeInfo> vtkF3DMetaImporter::GetActorHierarchy()
{
    std::vector<NodeInfo> nodes;

    // Return only the original imported actors, not internal F3D rendering actors
    vtkCollectionSimpleIterator ait;
    this->ActorCollection->InitTraversal(ait);
    while (vtkActor* actor = this->ActorCollection->GetNextActor(ait))
    {
        if (!actor)
            continue;

        NodeInfo node;
        node.prop = actor;
        // Use a more meaningful name if available, otherwise use class name
        node.name = actor->GetClassName();
        nodes.push_back(node);
    }

    std::cout << "[DEBUG] vtkF3DMetaImporter::GetActorHierarchy() collected "
              << nodes.size() << " original actors" << std::endl;

    return nodes;
}