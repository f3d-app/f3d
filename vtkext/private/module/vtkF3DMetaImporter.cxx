#include "vtkF3DMetaImporter.h"

#include "F3DLog.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DImporter.h"

#include <vtkActorCollection.h>
#include <vtkArrowSource.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkDataAssemblyVisitor.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkInformationIntegerKey.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
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

namespace
{

/**
 * Sets the `f3d_collapsed` attribute on nodes which have
 * all their children unnamed or named the same as themselves.
 * Allows to make the tree more compact on load by collapsing subtrees
 * that don't contain any meaningful user-provided labels.
 */
class vtkF3DCollapseOnLoadVisitor : public vtkDataAssemblyVisitor
{
public:
  static vtkF3DCollapseOnLoadVisitor* New();
  vtkTypeMacro(vtkF3DCollapseOnLoadVisitor, vtkDataAssemblyVisitor);

protected:
  void SetAttr(int nodeid, bool val)
  {
    vtkDataAssembly* mutableAssembly = const_cast<vtkDataAssembly*>(this->GetAssembly());
    mutableAssembly->SetAttribute(nodeid, "f3d_collapsed", val ? 1 : 0);
  }
  bool GetAttr(int nodeid)
  {
    return this->GetAssembly()->GetAttributeOrDefault(nodeid, "f3d_collapsed", 0) != 0;
  }

  void Visit(int nodeid) override
  {
    // don't collapse the root node
    if (nodeid == this->GetAssembly()->GetRootNode())
    {
      return;
    }

    const int numberOfChildren = this->GetAssembly()->GetNumberOfChildren(nodeid);
    std::vector<int> childrenIds;
    childrenIds.reserve(static_cast<size_t>(numberOfChildren));
    for (int childIndex = 0; childIndex < numberOfChildren; childIndex++)
    {
      childrenIds.emplace_back(this->GetAssembly()->GetChild(nodeid, childIndex));
    }

    const auto allChildrenAreUnnamed = [&]()
    {
      return std::none_of(childrenIds.cbegin(), childrenIds.cend(),
        [&](int id) { return this->GetAssembly()->HasAttribute(id, "label"); });
    };

    const auto allChildrenHaveSameNameAsNode = [&]()
    {
      const std::string_view nodeName =
        this->GetAssembly()->GetAttributeOrDefault(nodeid, "label", "");
      return std::all_of(childrenIds.cbegin(), childrenIds.cend(), [&](int id)
        { return nodeName == this->GetAssembly()->GetAttributeOrDefault(id, "label", ""); });
    };

    if (allChildrenAreUnnamed() || allChildrenHaveSameNameAsNode())
    {
      this->SetAttr(nodeid, true);
    }
  }

  void EndSubTree(int nodeid) override
  {
    // after all descendents have been visited, unset the attr if not all children have it set
    if (this->GetAttr(nodeid))
    {
      const int numberOfChildren = this->GetAssembly()->GetNumberOfChildren(nodeid);
      for (int childIndex = 0; childIndex < numberOfChildren; childIndex++)
      {
        if (!GetAttr(this->GetAssembly()->GetChild(nodeid, childIndex)))
        {
          this->SetAttr(nodeid, false);
          break;
        }
      }
    }
  }
};
vtkStandardNewMacro(vtkF3DCollapseOnLoadVisitor);
}

//----------------------------------------------------------------------------
struct vtkF3DMetaImporter::Internals
{
  // Actors related vectors
  std::vector<vtkF3DMetaImporter::ColoringStruct> ColoringActorsAndMappers;
  std::vector<vtkF3DMetaImporter::NormalGlyphsStruct> NormalGlyphsActorsAndMappers;
  std::vector<vtkF3DMetaImporter::PointSpritesStruct> PointSpritesActorsAndMappers;
  std::vector<vtkF3DMetaImporter::VolumeStruct> VolumePropsAndMappers;

  std::vector<vtkF3DMetaImporter::ImporterInfo> Importers;
  std::optional<vtkIdType> CameraIndex;
  vtkBoundingBox GeometryBoundingBox;
  vtkTimeStamp ColoringInfoTime;
  vtkTimeStamp UpdateTime;

  F3DColoringInfoHandler ColoringInfoHandler;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DMetaImporter);

//----------------------------------------------------------------------------
vtkInformationKeyMacro(vtkF3DMetaImporter, ACTOR_HIDDEN, Integer);

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
void vtkF3DMetaImporter::AddImporter(
  const std::pair<std::string, vtkSmartPointer<vtkImporter>>& importer)
{
  this->Pimpl->Importers.emplace_back(vtkF3DMetaImporter::ImporterInfo{
    importer.first, importer.second, false, vtkSmartPointer<vtkDataAssembly>::New() });
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
  importer.second->AddObserver(vtkCommand::ProgressEvent, progressCallback);
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
const std::vector<vtkF3DMetaImporter::NormalGlyphsStruct>&
vtkF3DMetaImporter::GetNormalGlyphsActorsAndMappers()
{
  return this->Pimpl->NormalGlyphsActorsAndMappers;
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
int vtkF3DMetaImporter::GetImporterInfoCount()
{
  return static_cast<int>(this->Pimpl->Importers.size());
}

//----------------------------------------------------------------------------
vtkF3DMetaImporter::ImporterInfo vtkF3DMetaImporter::GetImporterInfo(int index)
{
  return this->Pimpl->Importers[index];
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

  for (auto& importerInfo : this->Pimpl->Importers)
  {
    vtkImporter* importer = importerInfo.Importer;

    // Importer has already been updated
    if (importerInfo.Updated)
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

    if (!importer->Update())
    {
      return false;
    }

    localCameraIndex -= importer->GetNumberOfCameras();

    vtkActorCollection* actorCollection = importer->GetImportedActors();

    // copy the scene hierarchy if it exists, or create a generic one otherwise
    if (importer->GetSceneHierarchy() != nullptr)
    {
      importerInfo.DataAssembly->DeepCopy(importer->GetSceneHierarchy());
    }
    else
    {
      // add one node per actor
      for (int actorIndex = 0; actorIndex < actorCollection->GetNumberOfItems(); actorIndex++)
      {
        std::string actorName = "object" + std::to_string(actorIndex);
        const int nodeid = importerInfo.DataAssembly->AddNode(
          actorName.c_str(), importerInfo.DataAssembly->GetRootNode());
        importerInfo.DataAssembly->SetAttribute(nodeid, "flat_actor_id", actorIndex);
      }
    }

    importerInfo.DataAssembly->SetAttribute(
      vtkDataAssembly::GetRootNode(), "label", importerInfo.Name.c_str());

    vtkNew<::vtkF3DCollapseOnLoadVisitor> visitor;
    importerInfo.DataAssembly->Visit(vtkDataAssembly::GetRootNode(), visitor);
    // Unset the attr on all nodes which have an ancestor that has it already.
    // This avoids having to expand the collapsed levels one by one.
    const std::string xpath = "//*[@f3d_collapsed='1']//*[@f3d_collapsed='1']";
    for (const int nodeid : importerInfo.DataAssembly->SelectNodes({ xpath }))
    {
      importerInfo.DataAssembly->SetAttribute(nodeid, "f3d_collapsed", 0);
    }

    // Recover generic importer if any (for indexed access to points/image)
    vtkF3DGenericImporter* genericImporter = vtkF3DGenericImporter::SafeDownCast(importer);
    vtkIdType actorIndex = 0;

    vtkCollectionSimpleIterator ait;
    actorCollection->InitTraversal(ait);
    while (vtkActor* actor = actorCollection->GetNextActor(ait))
    {
      // Check for actor's poly data mapper, skip if none exists
      vtkPolyDataMapper* pdMapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
      if (pdMapper == nullptr)
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, "Actor has no mapped poly data and will not be rendered.");
        continue;
      }

      // Add to the actor collection
      this->ActorCollection->AddItem(actor);

      vtkPolyData* surface = pdMapper->GetInput();

      // convert to PBR materials if needed
      // this should be moved elsewhere, see https://github.com/f3d-app/f3d/issues/2995
      if (!genericImporter && actor->GetProperty()->GetInterpolation() != VTK_PBR &&
        actor->GetProperty()->GetLighting())
      {
        actor->GetProperty()->SetInterpolationToPBR();

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

      vtkPolyData* points = surface;
      if (genericImporter)
      {
        // Use indexed accessor for composite support
        points = genericImporter->GetImportedPoints(actorIndex);
      }

      // Create and configure normal glyph actors
      this->Pimpl->NormalGlyphsActorsAndMappers.emplace_back(
        vtkF3DMetaImporter::NormalGlyphsStruct(actor, importer));
      vtkF3DMetaImporter::NormalGlyphsStruct& ngs =
        this->Pimpl->NormalGlyphsActorsAndMappers.back();

      ngs.InputDataHasNormals = points->GetPointData()->GetNormals() != nullptr;

      if (ngs.InputDataHasNormals)
      {
        vtkNew<vtkArrowSource> arrowSource;
        ngs.GlyphMapper->SetInputData(points);
        ngs.GlyphMapper->SetSourceConnection(arrowSource->GetOutputPort());
        ngs.GlyphMapper->SetOrientationModeToDirection();
        ngs.GlyphMapper->SetOrientationArray(vtkDataSetAttributes::NORMALS);
        ngs.GlyphMapper->ScalingOn();
        ngs.Actor->SetMapper(ngs.GlyphMapper);
        this->Renderer->AddActor(ngs.Actor);
        ngs.Actor->VisibilityOff();
      }

      // Create and configure point sprites actors
      this->Pimpl->PointSpritesActorsAndMappers.emplace_back(
        vtkF3DMetaImporter::PointSpritesStruct(actor, importer));
      vtkF3DMetaImporter::PointSpritesStruct& pss =
        this->Pimpl->PointSpritesActorsAndMappers.back();

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
          this->Pimpl->VolumePropsAndMappers.emplace_back(vtkF3DMetaImporter::VolumeStruct(actor));
          vtkF3DMetaImporter::VolumeStruct& vs = this->Pimpl->VolumePropsAndMappers.back();
          vs.Mapper->SetInputData(image);
          this->Renderer->AddVolume(vs.Prop);
          vs.Prop->VisibilityOff();
        }
      }

      actorIndex++;
    }

    importerInfo.Updated = true;
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
    std::string(), [](const std::string& a, const auto& importerInfo)
    { return a + "----------\n" + importerInfo.Importer->GetOutputsDescription(); });
  return description;
}

//----------------------------------------------------------------------------
vtkF3DImporter::AnimationSupportLevel vtkF3DMetaImporter::GetAnimationSupportLevel()
{
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 4, 20250507)
  vtkF3DImporter::AnimationSupportLevel levelAccum = vtkF3DImporter::AnimationSupportLevel::MULTI;
#else
  vtkImporter::AnimationSupportLevel levelAccum = vtkImporter::AnimationSupportLevel::NONE;
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    AnimationSupportLevel level = importerInfo.Importer->GetAnimationSupportLevel();
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
    [](vtkIdType a, const auto& importerInfo)
    {
      vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      std::string name = importerInfo.Importer->GetAnimationName(localAnimationIndex);
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      importerInfo.Importer->EnableAnimation(localAnimationIndex);
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      importerInfo.Importer->DisableAnimation(localAnimationIndex);
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
      return importerInfo.Importer->IsAnimationEnabled(localAnimationIndex);
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
    [](vtkIdType a, const auto& importerInfo)
    { return a + importerInfo.Importer->GetNumberOfCameras(); });
}

//----------------------------------------------------------------------------
std::string vtkF3DMetaImporter::GetCameraName(vtkIdType camIndex)
{
  vtkIdType localCameraIndex = camIndex;
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nCam = importerInfo.Importer->GetNumberOfCameras();
    if (localCameraIndex < nCam)
    {
      std::string name = importerInfo.Importer->GetCameraName(localCameraIndex);
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    vtkIdType nAnim = importerInfo.Importer->GetNumberOfAnimations();
    if (nAnim < 0)
    {
      nAnim = 0;
    }

    if (localAnimationIndex < nAnim)
    {
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 20251210)
      vtkF3DImporter* f3dImporter = vtkF3DImporter::SafeDownCast(importerInfo.Importer);
      if (f3dImporter)
      {
        return f3dImporter->GetTemporalInformation(
          localAnimationIndex, timeRange, nbTimeSteps, timeSteps);
      }
      else
      {
        return importerInfo.Importer->GetTemporalInformation(
          localAnimationIndex, 0, nbTimeSteps, timeRange, timeSteps);
      }
#else
      return importerInfo.Importer->GetTemporalInformation(
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
  for (const auto& importerInfo : this->Pimpl->Importers)
  {
    ret = ret && importerInfo.Importer->UpdateAtTimeValue(timeValue);
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
    for (const auto& importerInfo : this->Pimpl->Importers)
    {
      vtkActorCollection* actorCollection = importerInfo.Importer->GetImportedActors();

      // Recover generic importer if any (for indexed access to points/image)
      vtkF3DGenericImporter* genericImporter =
        vtkF3DGenericImporter::SafeDownCast(importerInfo.Importer);
      vtkIdType actorIndex = 0;

      vtkCollectionSimpleIterator ait;
      actorCollection->InitTraversal(ait);
      while (auto* actor = actorCollection->GetNextActor(ait))
      {
        vtkPolyDataMapper* pdMapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        // Check for actor's poly data mapper, skip if none exists
        if (pdMapper == nullptr)
        {
          F3DLog::Print(
            F3DLog::Severity::Warning, "Actor has no mapped poly data and will not be colored.");
          continue;
        }

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
