#include "vtkF3DRendererWithColoring.h"

#include "F3DLog.h"
#include "vtkF3DConfigure.h"
#include "vtkF3DMetaImporter.h"

#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointGaussianMapper.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>
#include <vtkShader.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTexture.h>
#include <vtkVolumeProperty.h>
#include <vtksys/SystemTools.hxx>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <cassert>
#include <sstream>

namespace
{
//----------------------------------------------------------------------------
// TODO : add this function in a utils file for rendering in VTK directly
vtkSmartPointer<vtkTexture> GetTexture(const std::string& filePath, bool isSRGB = false)
{
  vtkSmartPointer<vtkTexture> texture;
  if (!filePath.empty())
  {
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(filePath);
    if (!vtksys::SystemTools::FileExists(fullPath))
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Texture file does not exist " + fullPath + "\n");
    }
    else
    {
      auto reader = vtkSmartPointer<vtkImageReader2>::Take(
        vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));
      if (reader)
      {
        reader->SetFileName(fullPath.c_str());
        reader->Update();
        texture = vtkSmartPointer<vtkTexture>::New();
        texture->SetInputConnection(reader->GetOutputPort());
        if (isSRGB)
        {
          texture->UseSRGBColorSpaceOn();
        }
        texture->InterpolateOn();
        texture->SetColorModeToDirectScalars();
        return texture;
      }
      else
      {
        F3DLog::Print(F3DLog::Severity::Warning, "Cannot open texture file " + fullPath + "\n");
      }
    }
  }

  return texture;
}
}

vtkStandardNewMacro(vtkF3DRendererWithColoring);

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetImporter(vtkF3DMetaImporter* importer)
{
  this->Importer = importer;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::Initialize()
{
  // This remove all actors and view props
  this->Superclass::Initialize();

  this->ArrayIndexForColoring = -1;
  this->ComponentForColoring = -1;

  this->AddActor2D(this->ScalarBarActor);
  this->ScalarBarActor->VisibilityOff();

  this->ColorTransferFunctionConfigured = false;
  this->ColoringMappersConfigured = false;
  this->PointSpritesMappersConfigured = false;
  this->VolumePropsAndMappersConfigured = false;
  this->ScalarBarActorConfigured = false;
  this->CheatSheetConfigured = false;
  this->ColoringActorsPropertiesConfigured = false;
  this->ColoringConfigured = false;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetRoughness(const std::optional<double>& roughness)
{
  if (this->Roughness != roughness)
  {
    this->Roughness = roughness;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetOpacity(const std::optional<double>& opacity)
{
  if (this->Opacity != opacity)
  {
    this->Opacity = opacity;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetMetallic(const std::optional<double>& metallic)
{
  if (this->Metallic != metallic)
  {
    this->Metallic = metallic;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetNormalScale(const std::optional<double>& normalScale)
{
  if (this->NormalScale != normalScale)
  {
    this->NormalScale = normalScale;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetSurfaceColor(const std::optional<std::vector<double>>& color)
{
  if (this->SurfaceColor != color)
  {
    this->SurfaceColor = color;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetEmissiveFactor(const std::optional<std::vector<double>>& factor)
{
  if (this->EmissiveFactor != factor)
  {
    this->EmissiveFactor = factor;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetTextureMatCap(const std::optional<std::string>& tex)
{
  if (this->TextureMatCap != tex)
  {
    this->TextureMatCap = tex;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetTextureBaseColor(const std::optional<std::string>& tex)
{
  if (this->TextureBaseColor != tex)
  {
    this->TextureBaseColor = tex;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetTextureMaterial(const std::optional<std::string>& tex)
{
  if (this->TextureMaterial != tex)
  {
    this->TextureMaterial = tex;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetTextureEmissive(const std::optional<std::string>& tex)
{
  if (this->TextureEmissive != tex)
  {
    this->TextureEmissive = tex;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetTextureNormal(const std::optional<std::string>& tex)
{
  if (this->TextureNormal != tex)
  {
    this->TextureNormal = tex;
    this->ColoringActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureColoringActorsProperties()
{
  assert(this->Importer);

  double* surfaceColor = nullptr;
  if (this->SurfaceColor.has_value())
  {
    if (this->SurfaceColor.value().size() != 3)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Invalid surface color provided, not applying\n"));
    }
    else
    {
      surfaceColor = this->SurfaceColor.value().data();
    }
  }

  double* emissiveFactor = nullptr;
  if (this->EmissiveFactor.has_value())
  {
    if (this->EmissiveFactor.value().size() != 3)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Invalid emissive factor provided, not applying\n"));
    }
    else
    {
      emissiveFactor = this->EmissiveFactor.value().data();
    }
  }

  for ([[maybe_unused]] const auto& [actor, mapper, originalActor] : this->Importer->GetColoringActorsAndMappers())
  {
    if(surfaceColor)
    {
      actor->GetProperty()->SetColor(surfaceColor);
      originalActor->GetProperty()->SetColor(surfaceColor);
    }

    if (this->Opacity.has_value())
    {
      actor->GetProperty()->SetOpacity(this->Opacity.value());
      originalActor->GetProperty()->SetOpacity(this->Opacity.value());
    }

    if (this->Roughness.has_value())
    {
      actor->GetProperty()->SetRoughness(this->Roughness.value());
      originalActor->GetProperty()->SetRoughness(this->Roughness.value());
    }

    if (this->Metallic.has_value())
    {
      actor->GetProperty()->SetMetallic(this->Metallic.value());
      originalActor->GetProperty()->SetMetallic(this->Metallic.value());
    }

    // Textures
    if (this->TextureBaseColor.has_value())
    {
      auto colorTex = ::GetTexture(this->TextureBaseColor.value(), true);
      actor->GetProperty()->SetBaseColorTexture(colorTex);
      originalActor->GetProperty()->SetBaseColorTexture(colorTex);

      // If the input texture is RGBA, flag the actor as translucent
      if (colorTex && colorTex->GetImageDataInput(0)->GetNumberOfScalarComponents() == 4)
      {
        actor->ForceTranslucentOn();
        originalActor->ForceTranslucentOn();
      }
    }

    if (this->TextureMaterial.has_value())
    {
      auto matTex = ::GetTexture(this->TextureMaterial.value());
      actor->GetProperty()->SetORMTexture(matTex);
      originalActor->GetProperty()->SetORMTexture(matTex);
    }

    if (this->TextureEmissive.has_value())
    {
      auto emissTex = ::GetTexture(this->TextureEmissive.value(), true);
      actor->GetProperty()->SetEmissiveTexture(emissTex);
      originalActor->GetProperty()->SetEmissiveTexture(emissTex);
    }

    if (emissiveFactor)
    {
      actor->GetProperty()->SetEmissiveFactor(emissiveFactor);
      originalActor->GetProperty()->SetEmissiveFactor(emissiveFactor);
    }

    if (this->TextureNormal.has_value())
    {
      auto normTex = ::GetTexture(this->TextureNormal.value());
      actor->GetProperty()->SetNormalTexture(normTex);
      originalActor->GetProperty()->SetNormalTexture(normTex);
    }

    if (this->NormalScale.has_value())
    {
      actor->GetProperty()->SetNormalScale(this->NormalScale.value());
      originalActor->GetProperty()->SetNormalScale(this->NormalScale.value());
    }

    if (this->TextureMatCap.has_value())
    {
      auto matCapTex = ::GetTexture(this->TextureMatCap.value());
      actor->GetProperty()->SetTexture("matcap", matCapTex);
      originalActor->GetProperty()->SetTexture("matcap", matCapTex);
    }
  }

  for ([[maybe_unused]] const auto& [actor, mapper] : this->Importer->GetPointSpritesActorsAndMappers())
  {
    if(surfaceColor)
    {
      actor->GetProperty()->SetColor(surfaceColor);
    }

    if (this->Opacity.has_value())
    {
      actor->GetProperty()->SetOpacity(this->Opacity.value());
    }
  }

  this->ColoringActorsPropertiesConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetPointSpritesProperties(SplatType type, double pointSpritesSize)
{
  assert(this->Importer);

  if (type == SplatType::GAUSSIAN)
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231102)
    if (!vtkShader::IsComputeShaderSupported())
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Compute shaders are not supported, gaussians are not sorted, resulting in blending "
        "artifacts");
    }
#endif
  }

  const vtkBoundingBox& bbox = this->Importer->GetGeometryBoundingBox();

  double scaleFactor = 1.0;
  if (bbox.IsValid())
  {
    scaleFactor = pointSpritesSize * bbox.GetDiagonalLength() * 0.001;
  }

  for (const auto& [actor, mapper] : this->Importer->GetPointSpritesActorsAndMappers())
  {

    mapper->EmissiveOff();
    if (type == SplatType::GAUSSIAN)
    {
      mapper->SetScaleFactor(1.0);
      mapper->SetSplatShaderCode(nullptr); // gaussian is the default VTK shader
      mapper->SetScaleArray("scale");

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231102)
      mapper->AnisotropicOn();
      mapper->SetBoundScale(3.0);
      mapper->SetRotationArray("rotation");

      int* viewport = this->GetSize();

      float lowPass[3] = { 0.3f / (viewport[0] * viewport[0]), 0.f,
        0.3f / (viewport[1] * viewport[1]) };
      mapper->SetLowpassMatrix(lowPass);
#else
      F3DLog::Print(F3DLog::Severity::Warning,
        "Gaussian splatting selected but VTK <= 9.3 only supports isotropic gaussians");
#endif

      actor->ForceTranslucentOn();
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231102)
      mapper->AnisotropicOff();
      mapper->SetLowpassMatrix(0., 0., 0.);
#endif

      mapper->SetScaleFactor(scaleFactor);

      mapper->SetSplatShaderCode("//VTK::Color::Impl\n"
                                 "float dist = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
                                 "if (dist > 1.0) {\n"
                                 "  discard;\n"
                                 "} else {\n"
                                 "  float scale = (1.0 - dist);\n"
                                 "  ambientColor *= scale;\n"
                                 "  diffuseColor *= scale;\n"
                                 "}\n");

      actor->ForceTranslucentOff();
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ShowScalarBar(bool show)
{
  if (this->ScalarBarVisible != show)
  {
    this->ScalarBarVisible = show;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUsePointSprites(bool use)
{
  if (this->UsePointSprites != use)
  {
    this->UsePointSprites = use;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseVolume(bool use)
{
  if (this->UseVolume != use)
  {
    this->UseVolume = use;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseInverseOpacityFunction(bool use)
{
  assert(this->Importer);

  if (this->UseInverseOpacityFunction != use)
  {
    this->UseInverseOpacityFunction = use;
    for ([[maybe_unused]] const auto& [prop, mapper] : this->Importer->GetVolumePropsAndMappers())
    {
      if (prop)
      {
        vtkPiecewiseFunction* pwf = prop->GetProperty()->GetScalarOpacity();
        if (pwf->GetSize() == 2)
        {
          double range[2];
          pwf->GetRange(range);

          pwf->RemoveAllPoints();
          pwf->AddPoint(range[0], this->UseInverseOpacityFunction ? 1.0 : 0.0);
          pwf->AddPoint(range[1], this->UseInverseOpacityFunction ? 0.0 : 1.0);
        }
      }
    }
    this->VolumePropsAndMappersConfigured = false;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetScalarBarRange(const std::optional<std::vector<double>>& range)
{
  if (this->UserScalarBarRange != range)
  {
    this->UserScalarBarRange = range;
    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColormap(const std::vector<double>& colormap)
{
  if (this->Colormap != colormap)
  {
    this->Colormap = colormap;

    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;

    this->ScalarBarActorConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleScalars(CycleType type)
{
  switch (type)
  {
    case (CycleType::NONE):
      return;
      break;
    case (CycleType::FIELD):
      this->CycleFieldForColoring();
      break;
    case (CycleType::ARRAY_INDEX):
      this->CycleArrayIndexForColoring();
      break;
    case (CycleType::COMPONENT):
      this->CycleComponentForColoring();
      break;
    default:
      break;
  }

  // Check attributes are valid and cycle recursively if needed
  this->CycleScalars(this->CheckColoring());

  this->ColorTransferFunctionConfigured = false;
  this->ColoringMappersConfigured = false;
  this->PointSpritesMappersConfigured = false;
  this->VolumePropsAndMappersConfigured = false;
  this->ScalarBarActorConfigured = false;
  this->CheatSheetConfigured = false;
  this->ColoringConfigured = false;
}

//----------------------------------------------------------------------------
vtkF3DRendererWithColoring::CycleType vtkF3DRendererWithColoring::CheckColoring()
{
  assert(this->Importer);

  // Never force change of anything if we are currently not coloring
  if (this->ArrayIndexForColoring < 0)
  {
    return CycleType::NONE;
  }

  // Never force change of CellData/PointData coloring on the user
  if (this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring) == 0)
  {
    return CycleType::NONE;
  }

  // Suggest to change the array index only if current index is not valid
  vtkF3DMetaImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return CycleType::ARRAY_INDEX;
  }

  // Suggest to change the component if current component is invalid
  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    return CycleType::COMPONENT;
  }

  return CycleType::NONE;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColoring(bool enable,
  bool useCellData, const std::optional<std::string>& arrayName, int component)
{
  assert(this->Importer);

  // XXX This should be reworked to avoid handling multiple information in one parameters
  // while still being future-proof and flexible enough.
  if (enable != (this->ArrayIndexForColoring >= 0)
      || useCellData != this->UseCellColoring
      || component != this->ComponentForColoring
      || arrayName != this->GetColoringArrayName())
  {
    this->UseCellColoring = useCellData;
    this->ComponentForColoring = component;

    int nIndexes = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring);
    if (!enable)
    {
      // Not coloring
      this->ArrayIndexForColoring = -1;
    }
    else if (nIndexes == 0)
    {
      // Trying to color but no array available
      F3DLog::Print(F3DLog::Severity::Debug, "No array to color with");
      this->ArrayIndexForColoring = -1;
    }
    else if (!arrayName.has_value())
    {
      // Coloring with first array
      this->ArrayIndexForColoring = 0;
    }
    else
    {
      // Coloring with named array
      this->ArrayIndexForColoring = this->Importer->FindIndexForColoring(useCellData, arrayName.value());
      if (this->ArrayIndexForColoring == -1)
      {
        // Could not find named array
        F3DLog::Print(F3DLog::Severity::Warning, "Unknown scalar array: \"" + arrayName.value() + "\"\n");
      }
    }

    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::GetColoringEnabled()
{
  return this->ArrayIndexForColoring >= 0;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::GetColoringUseCell()
{
  return this->UseCellColoring;
}

//----------------------------------------------------------------------------
std::optional<std::string> vtkF3DRendererWithColoring::GetColoringArrayName()
{
  assert(this->Importer);

  std::optional<std::string> arrayName;
  vtkF3DMetaImporter::ColoringInfo info;
  if (this->Importer && this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    arrayName = info.Name;
  }
  return arrayName;
}

//----------------------------------------------------------------------------
int vtkF3DRendererWithColoring::GetColoringComponent()
{
  return this->ComponentForColoring;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::UpdateActors()
{
  assert(this->Importer);

  // Handle importer changes
  // XXX: Importer only modify itself when adding a new importer,
  // not when updating at a time step
  vtkMTimeType importerMTime = this->Importer->GetMTime();
  bool importerChanged = this->Importer->GetMTime() > this->ImporterTimeStamp;
  if (importerChanged)
  {
    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ActorsPropertiesConfigured = false;
    this->GridConfigured = false;
    this->MetaDataConfigured = false;
    this->ColoringActorsPropertiesConfigured = false;
    this->ColoringConfigured = false;
  }
  this->ImporterTimeStamp = importerMTime;

  if (!this->ColoringActorsPropertiesConfigured)
  {
    this->ConfigureColoringActorsProperties();
  }

  if (!this->ColoringConfigured)
  {
    this->ConfigureColoring();
  }

  // Call superclass update actors after everything,
  // as the grid need all actors visible to be configured correctly
  this->Superclass::UpdateActors();
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureColoring()
{
  assert(this->Importer);

  // Recover coloring information
  vtkF3DMetaImporter::ColoringInfo info;
  bool hasColoring =
    this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  bool volumeVisible = !this->UseRaytracing && this->UseVolume;
  if (!hasColoring && volumeVisible)
  {
    // When showing volume, always try to find an array to color with
    this->CycleScalars(vtkF3DRendererWithColoring::CycleType::ARRAY_INDEX);
    hasColoring =
      this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);
  }

  if (hasColoring && !this->ColorTransferFunctionConfigured)
  {
    this->ConfigureRangeAndCTFForColoring(info);
    this->ColorTransferFunctionConfigured = true;
  }

  // Handle surface geometry
  bool geometriesVisible = this->UseRaytracing || (!this->UseVolume && !this->UsePointSprites);
  for (const auto& [actor, mapper, originalActor] : this->Importer->GetColoringActorsAndMappers())
  {
    if (geometriesVisible)
    {
      bool visible = false;
      if (hasColoring)
      {
        // Rely on the previous state of scalar visibility to know if we should show the actor by default
        visible = mapper->GetScalarVisibility();
        if (!this->ColoringMappersConfigured)
        {
          visible = vtkF3DRendererWithColoring::ConfigureMapperForColoring(mapper, info.Name,
            this->ComponentForColoring, this->ColorTransferFunction, this->ColorRange,
            this->UseCellColoring);
        }
      }
      actor->SetVisibility(visible);
      originalActor->SetVisibility(!visible);
    }
    else
    {
      actor->SetVisibility(false);
      originalActor->SetVisibility(false);
    }
  }
  if (geometriesVisible)
  {
    this->ColoringMappersConfigured = true;
  }

  // Handle point sprites
  bool pointSpritesVisible = !this->UseRaytracing && !this->UseVolume && this->UsePointSprites;
  for (const auto& [actor, mapper] : this->Importer->GetPointSpritesActorsAndMappers())
  {
    actor->SetVisibility(pointSpritesVisible);
    if (pointSpritesVisible)
    {
      if (hasColoring)
      {
        if (!this->PointSpritesMappersConfigured)
        {
          vtkF3DRendererWithColoring::ConfigureMapperForColoring(mapper, info.Name,
            this->ComponentForColoring, this->ColorTransferFunction, this->ColorRange,
            this->UseCellColoring);
        }
      }
      mapper->SetScalarVisibility(hasColoring);
    }
  }
  if (pointSpritesVisible)
  {
    this->PointSpritesMappersConfigured = true;
  }

  // Handle Volume prop
  const auto& volPropsAndMappers = this->Importer->GetVolumePropsAndMappers();
  for (const auto& [prop, mapper] : volPropsAndMappers)
  {
    if (!volumeVisible)
    {
      prop->VisibilityOff();
    }
    else
    {
      bool visible = false;
      if (hasColoring)
      {
        // Initialize the visibility based on the mapper configuration
        visible = !std::string(mapper->GetArrayName()).empty();
        if (!this->VolumePropsAndMappersConfigured)
        {
          visible = vtkF3DRendererWithColoring::ConfigureVolumeForColoring(mapper,
            prop, info.Name, this->ComponentForColoring,
            this->ColorTransferFunction, this->ColorRange, this->UseCellColoring,
            this->UseInverseOpacityFunction);
          if (!visible)
          {
            F3DLog::Print(
              F3DLog::Severity::Warning, "Cannot find the array \"" + info.Name + "\" to display volume with");
          }
        }
      }
      prop->SetVisibility(visible);
    }
  }
  if (volumeVisible)
  {
    if (!this->VolumePropsAndMappersConfigured && volPropsAndMappers.size() == 0)
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Cannot use volume with this data");
    }
    this->VolumePropsAndMappersConfigured = true;
  }

  // Handle scalar bar
  bool barVisible = this->ScalarBarVisible && hasColoring && this->ComponentForColoring >= -1;
  this->ScalarBarActor->SetVisibility(barVisible);
  if (barVisible && !this->ScalarBarActorConfigured)
  {
    vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(
      this->ScalarBarActor, info.Name, this->ComponentForColoring, this->ColorTransferFunction);
    this->ScalarBarActorConfigured = true;
  }

  this->RenderPassesConfigured = false;
  this->ColoringConfigured = true;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GetColoringDescription()
{
  assert(this->Importer);

  std::stringstream stream;
  vtkF3DMetaImporter::ColoringInfo info;
  if (this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    stream << "Coloring using " << (this->UseCellColoring ? "cell" : "point") << " array named "
           << info.Name << ", "
           << vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring) << "\n";
  }
  else
  {
    stream << "Not coloring\n";
  }
  return stream.str();
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::ConfigureMapperForColoring(vtkPolyDataMapper* mapper, const std::string& name,
  int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag)
{
  vtkDataSetAttributes* data = cellFlag ?
    static_cast<vtkDataSetAttributes*>(mapper->GetInput()->GetCellData()) :
    static_cast<vtkDataSetAttributes*>(mapper->GetInput()->GetPointData());
  vtkDataArray* array = data->GetArray(name.c_str());
  if (!array || component >= array->GetNumberOfComponents())
  {
    mapper->ScalarVisibilityOff();
    return false;
  }

  mapper->SetColorModeToMapScalars();
  mapper->SelectColorArray(name.c_str());
  mapper->SetScalarMode(
    cellFlag ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
  mapper->ScalarVisibilityOn();

  if (component == -2)
  {
    if (array->GetNumberOfComponents() > 4)
    {
      // comp > 4 is actually not supported and would fail with a vtk error
      F3DLog::Print(F3DLog::Severity::Warning,
        "Direct scalars rendering not supported by array with more than 4 components");
      return false;
    }
    else
    {
      mapper->SetColorModeToDirectScalars();
    }
  }
  else
  {
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarRange(range);
    mapper->SetLookupTable(ctf);
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper,
  vtkVolume* volume, const std::string& name, int component, vtkColorTransferFunction* ctf,
  double range[2], bool cellFlag, bool inverseOpacityFlag)
{
  vtkDataSetAttributes* data = cellFlag ?
    static_cast<vtkDataSetAttributes*>(mapper->GetInput()->GetCellData()) :
    static_cast<vtkDataSetAttributes*>(mapper->GetInput()->GetPointData());
  vtkDataArray* array = data->GetArray(name.c_str());
  if (!array || component >= array->GetNumberOfComponents())
  {
    // We rely on the selected scalar array to check if this mapper can be shown or not
    mapper->SelectScalarArray("");
    return false;
  }

  mapper->SetScalarMode(
    cellFlag ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
  mapper->SelectScalarArray(name.c_str());

  if (component >= 0)
  {
    mapper->SetVectorMode(vtkSmartVolumeMapper::COMPONENT);
    mapper->SetVectorComponent(component);
  }
  else if (component == -1)
  {
    mapper->SetVectorMode(vtkSmartVolumeMapper::MAGNITUDE);
  }
  else if (component == -2)
  {
    if (array->GetNumberOfComponents() > 4)
    {
      // comp > 4 is actually not supported and would fail with a vtk error
      F3DLog::Print(F3DLog::Severity::Warning,
        "Direct scalars rendering not supported by array with more than 4 components");
      return false;
    }
    else
    {
      mapper->SetVectorMode(vtkSmartVolumeMapper::DISABLED);
    }
  }

  vtkNew<vtkPiecewiseFunction> otf;
  otf->AddPoint(range[0], inverseOpacityFlag ? 1.0 : 0.0);
  otf->AddPoint(range[1], inverseOpacityFlag ? 0.0 : 1.0);

  vtkNew<vtkVolumeProperty> property;
  property->SetColor(ctf);
  property->SetScalarOpacity(otf);
  property->ShadeOff();
  property->SetInterpolationTypeToLinear();

  volume->SetProperty(property);
  return true;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(
  vtkScalarBarActor* scalarBar, std::string arrayName, int component, vtkColorTransferFunction* ctf)
{
  arrayName += " (";
  arrayName += this->ComponentToString(component);
  arrayName += ")";

  scalarBar->SetLookupTable(ctf);
  scalarBar->SetTitle(arrayName.c_str());
  scalarBar->SetNumberOfLabels(4);
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetWidth(0.8);
  scalarBar->SetHeight(0.07);
  scalarBar->SetPosition(0.1, 0.01);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureRangeAndCTFForColoring(
  const vtkF3DMetaImporter::ColoringInfo& info)
{
  if (this->ComponentForColoring == -2)
  {
    return;
  }

  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      std::string("Invalid component index: ") + std::to_string(this->ComponentForColoring) + "\n");
    return;
  }

  // Set range
  bool autoRange = true;
  if (this->UserScalarBarRange.has_value())
  {
    if (this->UserScalarBarRange.value().size() == 2 && this->UserScalarBarRange.value()[0] <= this->UserScalarBarRange.value()[1])
    {
      autoRange = false;
      this->ColorRange[0] = this->UserScalarBarRange.value()[0];
      this->ColorRange[1] = this->UserScalarBarRange.value()[1];
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Invalid scalar range provided, using automatic range\n"));
    }
  }

  if (autoRange)
  {
    if (this->ComponentForColoring >= 0)
    {
      this->ColorRange[0] = info.ComponentRanges[this->ComponentForColoring][0];
      this->ColorRange[1] = info.ComponentRanges[this->ComponentForColoring][1];
    }
    else
    {
      this->ColorRange[0] = info.MagnitudeRange[0];
      this->ColorRange[1] = info.MagnitudeRange[1];
    }
  }

  // Create lookup table
  this->ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  if (this->Colormap.size() > 0)
  {
    if (this->Colormap.size() % 4 == 0)
    {
      for (size_t i = 0; i < this->Colormap.size(); i += 4)
      {
        double val = this->Colormap[i];
        double r = this->Colormap[i + 1];
        double g = this->Colormap[i + 2];
        double b = this->Colormap[i + 3];
        this->ColorTransferFunction->AddRGBPoint(
          this->ColorRange[0] + val * (this->ColorRange[1] - this->ColorRange[0]), r, g, b);
      }
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Specified color map list count is not a multiple of 4, ignoring it.");
    }
  }

  if (this->ComponentForColoring >= 0)
  {
    this->ColorTransferFunction->SetVectorModeToComponent();
    this->ColorTransferFunction->SetVectorComponent(this->ComponentForColoring);
  }
  else
  {
    this->ColorTransferFunction->SetVectorModeToMagnitude();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
  assert(this->Importer);

  vtkF3DMetaImporter::ColoringInfo info;
  bool hasColoring =
    this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  cheatSheetText << " C: Cell scalars coloring [" << (this->UseCellColoring ? "ON" : "OFF")
                 << "]\n";
  cheatSheetText << " S: Scalars coloring ["
                 << (hasColoring ? vtkF3DRenderer::ShortName(info.Name, 19) : "OFF") << "]\n";
  cheatSheetText << " Y: Coloring component ["
                 << vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring)
                 << "]\n";
  cheatSheetText << " B: Scalar bar " << (this->ScalarBarVisible ? "[ON]" : "[OFF]") << "\n";

  cheatSheetText << " V: Volume representation " << (this->UseVolume ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " I: Inverse volume opacity "
                 << (this->UseInverseOpacityFunction ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " O: Point sprites " << (this->UsePointSprites ? "[ON]" : "[OFF]") << "\n";
  this->Superclass::FillCheatSheetHotkeys(cheatSheetText);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleFieldForColoring()
{
  // A generic approach will be better when adding categorical field data coloring
  this->UseCellColoring = !this->UseCellColoring;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleArrayIndexForColoring()
{
  assert(this->Importer);

  int nIndex = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring);
  if (nIndex <= 0)
  {
    return;
  }

  if (this->UseVolume)
  {
    this->ArrayIndexForColoring = (this->ArrayIndexForColoring + 1) % nIndex;
  }
  else
  {
    // Cycle through arrays looping back to -1
    // -1 0 1 2 -1 0 1 2 ...
    this->ArrayIndexForColoring = (this->ArrayIndexForColoring + 2) % (nIndex + 1) - 1;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleComponentForColoring()
{
  assert(this->Importer);

  vtkF3DMetaImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return;
  }

  // -2 -1 0 1 2 3 4
  this->ComponentForColoring =
    (this->ComponentForColoring + 3) % (info.MaximumNumberOfComponents + 2) - 2;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GenerateMetaDataDescription()
{
  assert(this->Importer);

  // XXX Padding should not be handled by manipulating string
  // but on the actor directly, but it is not supported by VTK yet.

  // add eol before/after the string
  std::string description = "\n" + this->Importer->GetMetaDataDescription() + "\n";
  size_t index = 0;
  while (true)
  {
    index = description.find('\n', index);
    if (index == std::string::npos)
    {
      break;
    }
    // Add spaces after/before eol
    description.insert(index + 1, " ");
    description.insert(index, " ");
    index += 3;
  }

  return description;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::ComponentToString(int component)
{
  assert(this->Importer);

  if (component == -2)
  {
    return "Direct Scalars";
  }
  else if (component == -1)
  {
    return "Magnitude";
  }
  else
  {
    vtkF3DMetaImporter::ColoringInfo info;
    if (!this->Importer->GetInfoForColoring(
          this->UseCellColoring, this->ArrayIndexForColoring, info))
    {
      return "";
    }
    if (component >= info.MaximumNumberOfComponents)
    {
      return "";
    }

    std::string componentName;
    if (component < static_cast<int>(info.ComponentNames.size()))
    {
      componentName = info.ComponentNames[component];
    }
    if (componentName.empty())
    {
      componentName = "Component #";
      componentName += std::to_string(component);
    }
    return componentName;
  }
}
