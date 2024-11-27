#include "vtkF3DRenderer.h"

#include "F3DDefaultHDRI.h"
#include "F3DLog.h"
#include "F3DColoringInfoHandler.h"
#include "vtkF3DCachedLUTTexture.h"
#include "vtkF3DCachedSpecularTexture.h"
#include "vtkF3DConfigure.h"
#include "vtkF3DDropZoneActor.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderPass.h"
#include "vtkF3DUserRenderPass.h"

#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkCornerAnnotation.h>
#include <vtkCullerCollection.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkLightKit.h>
#include <vtkMath.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLTexture.h>
#include <vtkPBRLUTTexture.h>
#include <vtkPNGReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPixelBufferObject.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <vtkSkybox.h>
#include <vtkTable.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextureObject.h>
#include <vtkToneMappingPass.h>
#include <vtkVersion.h>
#include <vtkVolumeProperty.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLMultiBlockDataWriter.h>
#include <vtkXMLTableReader.h>
#include <vtkXMLTableWriter.h>
#include <vtksys/FStream.hxx>
#include <vtksys/MD5.h>
#include <vtksys/SystemTools.hxx>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
#include <vtkSphericalHarmonics.h>
#endif

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20220907)
#include <vtkOrientationMarkerWidget.h>
#else
#include "vtkF3DOrientationMarkerWidget.h"
#endif

#if F3D_MODULE_RAYTRACING
#include <vtkOSPRayRendererNode.h>
#endif

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

#include <cctype>
#include <chrono>
#include <regex>
#include <sstream>

namespace
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
//----------------------------------------------------------------------------
// Compute the MD5 hash of an existing file on disk
std::string ComputeFileHash(const std::string& filepath)
{
  unsigned char digest[16];
  char md5Hash[33];
  md5Hash[32] = '\0';

  std::size_t length = vtksys::SystemTools::FileLength(filepath);
  std::vector<char> buffer(length);

  vtksys::ifstream file;
  file.open(filepath.c_str(), std::ios_base::binary);
  file.read(buffer.data(), length);

  vtksysMD5* md5 = vtksysMD5_New();
  vtksysMD5_Initialize(md5);
  vtksysMD5_Append(
    md5, reinterpret_cast<const unsigned char*>(buffer.data()), static_cast<int>(length));
  vtksysMD5_Finalize(md5, digest);
  vtksysMD5_DigestToHex(digest, md5Hash);
  vtksysMD5_Delete(md5);

  return md5Hash;
}

#ifndef __EMSCRIPTEN__
//----------------------------------------------------------------------------
// Download texture from the GPU to a vtkImageData
vtkSmartPointer<vtkImageData> SaveTextureToImage(
  vtkTextureObject* tex, unsigned int target, unsigned int level, unsigned int size, int type)
{
  unsigned int dims[2] = { size, size };
  vtkIdType incr[2] = { 0, 0 };

  unsigned int nbFaces = tex->GetTarget() == GL_TEXTURE_CUBE_MAP ? 6 : 1;

  vtkNew<vtkImageData> img;
  img->SetDimensions(size, size, nbFaces);
  img->AllocateScalars(type, tex->GetComponents());

  for (unsigned int i = 0; i < nbFaces; i++)
  {
    vtkPixelBufferObject* pbo = tex->Download(target + i, level);

    pbo->Download2D(type, img->GetScalarPointer(0, 0, i), dims, tex->GetComponents(), incr);
    pbo->Delete();
  }

  return img;
}
#endif
#endif

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
      F3DLog::Print(F3DLog::Severity::Warning, "Texture file does not exist " + fullPath);
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
        F3DLog::Print(F3DLog::Severity::Warning, "Cannot open texture file " + fullPath);
      }
    }
  }

  return texture;
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DRenderer);

//----------------------------------------------------------------------------
vtkF3DRenderer::vtkF3DRenderer()
{
  this->Cullers->RemoveAllItems();
  this->AutomaticLightCreationOff();
  this->SetClippingRangeExpansion(0.99);

  // Create cached texture
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20230902)
  this->EnvMapLookupTable = vtkSmartPointer<vtkF3DCachedLUTTexture>::New();
  this->EnvMapPrefiltered = vtkSmartPointer<vtkF3DCachedSpecularTexture>::New();
#else
  this->EnvMapLookupTable = vtkF3DCachedLUTTexture::New();
  this->EnvMapPrefiltered = vtkF3DCachedSpecularTexture::New();
#endif
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
  this->EnvMapPrefiltered->HalfPrecisionOff();
#endif

  // Init actors
  vtkNew<vtkTextProperty> textProp;
  textProp->SetFontSize(14);
  textProp->SetOpacity(1.0);
  textProp->SetBackgroundColor(0, 0, 0);
  textProp->SetBackgroundOpacity(0.8);

  this->MetaDataActor->SetTextProperty(textProp);

  this->TimerActor->GetTextProperty()->SetFontSize(15);
  this->TimerActor->SetPosition(10, 10);
  this->TimerActor->SetInput("0 fps");

  this->CheatSheetActor->SetTextProperty(textProp);

  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();
  this->DropZoneActor->GetTextProperty()->SetFontFamilyToCourier();

  this->SkyboxActor->SetProjection(vtkSkybox::Sphere);
  this->SkyboxActor->GammaCorrectOn();

  this->FilenameActor->VisibilityOff();
  this->MetaDataActor->VisibilityOff();
  this->TimerActor->VisibilityOff();
  this->CheatSheetActor->VisibilityOff();
  this->DropZoneActor->VisibilityOff();
  this->SkyboxActor->VisibilityOff();

  // Make sure an active camera is available on the renderer
  this->GetActiveCamera();
}

//----------------------------------------------------------------------------
vtkF3DRenderer::~vtkF3DRenderer() = default;

//----------------------------------------------------------------------------
void vtkF3DRenderer::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->Timer != 0)
  {
    glDeleteQueries(1, &this->Timer);
    this->Timer = 0;
  }
  this->Superclass::ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Initialize()
{
  this->OriginalLightIntensities.clear();
  this->RemoveAllViewProps();
  this->RemoveAllLights();

  this->AddActor(this->FilenameActor);
  this->AddActor(this->GridActor);
  this->AddActor(this->TimerActor);
  this->AddActor(this->MetaDataActor);
  this->AddActor(this->DropZoneActor);
  this->AddActor(this->CheatSheetActor);
  this->AddActor(this->SkyboxActor);

  this->GridConfigured = false;
  this->CheatSheetConfigured = false;
  this->ActorsPropertiesConfigured = false;
  this->RenderPassesConfigured = false;
  this->LightIntensitiesConfigured = false;
  this->TextActorsConfigured = false;
  this->MetaDataConfigured = false;
  this->HDRITextureConfigured = false;
  this->HDRILUTConfigured = false;
  this->HDRISphericalHarmonicsConfigured = false;
  this->HDRISpecularConfigured = false;
  this->HDRISkyboxConfigured = false;

  this->GridInfo = "";

  this->AddActor2D(this->ScalarBarActor);
  this->ScalarBarActor->VisibilityOff();

  this->ColorTransferFunctionConfigured = false;
  this->ColoringMappersConfigured = false;
  this->PointSpritesMappersConfigured = false;
  this->VolumePropsAndMappersConfigured = false;
  this->ScalarBarActorConfigured = false;
  this->CheatSheetConfigured = false;
  this->ColoringConfigured = false;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::InitializeUpVector(const std::string& upString)
{
  const std::regex re("([-+]?)([XYZ])", std::regex_constants::icase);
  std::smatch match;
  if (std::regex_match(upString, match, re))
  {
    const float sign = match[1].str() == "-" ? -1.0 : +1.0;
    const int index = std::toupper(match[2].str()[0]) - 'X';
    assert(index >= 0 && index < 3);

    this->UpIndex = index;

    std::fill(this->UpVector, this->UpVector + 3, 0);
    this->UpVector[this->UpIndex] = sign;

    std::fill(this->RightVector, this->RightVector + 3, 0);
    this->RightVector[this->UpIndex == 0 ? 1 : 0] = 1.0;

    double pos[3];
    vtkMath::Cross(this->UpVector, this->RightVector, pos);
    vtkMath::MultiplyScalar(pos, -1.0);

    // XXX: Initialize the camera to a default position
    // Note that camera reset is expected to be called later during importing
    vtkCamera* cam = this->GetActiveCamera();
    cam->SetFocalPoint(0.0, 0.0, 0.0);
    cam->SetPosition(pos);
    cam->SetViewUp(this->UpVector);

    // skybox orientation
    double front[3];
    vtkMath::Cross(this->RightVector, this->UpVector, front);
    this->SkyboxActor->SetFloorPlane(this->UpVector[0], this->UpVector[1], this->UpVector[2], 0.0);
    this->SkyboxActor->SetFloorRight(front[0], front[1], front[2]);

    // environment orientation
    this->SetEnvironmentUp(this->UpVector);
    this->SetEnvironmentRight(this->RightVector);
  }
  else
  {
    F3DLog::Print(F3DLog::Severity::Warning, upString + " is not a valid up direction");
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureRenderPasses()
{
  // clean up previous pass
  vtkRenderPass* pass = this->GetPass();
  if (pass)
  {
    pass->ReleaseGraphicsResources(this->RenderWindow);
  }

  vtkNew<vtkF3DRenderPass> newPass;
  newPass->SetUseRaytracing(F3D_MODULE_RAYTRACING && this->UseRaytracing);
  newPass->SetUseSSAOPass(this->UseSSAOPass);
  newPass->SetUseDepthPeelingPass(this->UseDepthPeelingPass);
  newPass->SetUseBlurBackground(this->UseBlurBackground);
  newPass->SetCircleOfConfusionRadius(this->CircleOfConfusionRadius);
  newPass->SetForceOpaqueBackground(this->HDRISkyboxVisible);

  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);
  newPass->SetBounds(bounds);

  // Image post processing passes
  vtkSmartPointer<vtkRenderPass> renderingPass = newPass;

  if (this->UseToneMappingPass)
  {
    vtkNew<vtkToneMappingPass> toneP;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240609)
    toneP->SetToneMappingType(vtkToneMappingPass::NeutralPBR);
#else
    toneP->SetToneMappingType(vtkToneMappingPass::GenericFilmic);
    toneP->SetGenericFilmicDefaultPresets();
#endif
    toneP->SetDelegatePass(renderingPass);
    renderingPass = toneP;
  }

  if (this->UseFXAAPass)
  {
    vtkNew<vtkOpenGLFXAAPass> fxaaP;
    fxaaP->SetDelegatePass(renderingPass);

    this->SetPass(fxaaP);
    renderingPass = fxaaP;
  }

  if (this->FinalShader.has_value())
  {
    // basic validation
    if (this->FinalShader.value().find("pixel") != std::string::npos)
    {
      vtkNew<vtkF3DUserRenderPass> userP;
      userP->SetUserShader(this->FinalShader.value().c_str());
      userP->SetDelegatePass(renderingPass);

      this->SetPass(userP);
      renderingPass = userP;
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Final shader must define a function named \"pixel\"");
    }
  }

  this->SetPass(renderingPass);

#if F3D_MODULE_RAYTRACING
  vtkOSPRayRendererNode::SetRendererType("pathtracer", this);
  vtkOSPRayRendererNode::SetSamplesPerPixel(this->RaytracingSamples, this);
  vtkOSPRayRendererNode::SetEnableDenoiser(this->UseRaytracingDenoiser, this);
  vtkOSPRayRendererNode::SetDenoiserThreshold(0, this);

  vtkOSPRayRendererNode::BackgroundMode mode = vtkOSPRayRendererNode::Backplate;
  if (this->GetUseImageBasedLighting())
  {
    mode = vtkOSPRayRendererNode::Both;
  }
  vtkOSPRayRendererNode::SetBackgroundMode(mode, this);
#else
  if (this->UseRaytracing || this->UseRaytracingDenoiser)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      "Raytracing options can't be used if F3D has not been built with raytracing");
  }
#endif
  this->RenderPassesConfigured = true;
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::GetSceneDescription()
{
  std::string descr;

  std::stringstream stream;

  // Bounding box
  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);

  stream << "Scene bounding box: " << bounds[0] << "," << bounds[1] << "," << bounds[2] << ","
         << bounds[3] << "," << bounds[4] << "," << bounds[5] << "\n\n";

  // Camera Info
  vtkCamera* cam = this->GetActiveCamera();
  double position[3];
  double focal[3];
  double up[3];
  cam->GetPosition(position);
  cam->GetFocalPoint(focal);
  cam->GetViewUp(up);

  stream << "Camera position: " << position[0] << "," << position[1] << "," << position[2] << "\n"
         << "Camera focal point: " << focal[0] << "," << focal[1] << "," << focal[2] << "\n"
         << "Camera view up: " << up[0] << "," << up[1] << "," << up[2] << "\n"
         << "Camera view angle: " << cam->GetViewAngle() << "\n\n";
  descr += stream.str();

  // Grid Info
  if (this->GridVisible)
  {
    descr += this->GridInfo;
  }
  return descr;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowAxis(bool show)
{
  // Dynamic visible axis
  // XXX this could be handled in UpdateActors
  // but it is not needed as axis actor is not impacted by
  // by any other parameters and require special
  // care when destructing this renderer
  if (this->AxisVisible != show)
  {
    this->AxisWidget = nullptr;
    if (show)
    {
      assert(this->RenderWindow->GetInteractor());
      vtkNew<vtkAxesActor> axes;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20220907)
      this->AxisWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
#else
      this->AxisWidget = vtkSmartPointer<vtkF3DOrientationMarkerWidget>::New();
#endif
      this->AxisWidget->SetOrientationMarker(axes);
      this->AxisWidget->SetInteractor(this->RenderWindow->GetInteractor());
      this->AxisWidget->SetViewport(0.85, 0.0, 1.0, 0.15);
      this->AxisWidget->On();
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20220907)
      this->AxisWidget->InteractiveOff();
#endif
      this->AxisWidget->SetKeyPressActivation(false);
    }

    this->AxisVisible = show;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetGridAbsolute(bool absolute)
{
  if (this->GridAbsolute != absolute)
  {
    this->GridAbsolute = absolute;
    this->GridConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetGridUnitSquare(const std::optional<double>& unitSquare)
{
  if (this->GridUnitSquare != unitSquare)
  {
    this->GridUnitSquare = unitSquare;
    this->GridConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetGridSubdivisions(int subdivisions)
{
  if (this->GridSubdivisions != subdivisions)
  {
    this->GridSubdivisions = subdivisions;
    this->GridConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetGridColor(const std::vector<double>& color)
{
  assert(color.size() == 3);

  if (this->GridColor[0] != color[0] || this->GridColor[1] != color[1] || this->GridColor[2] != color[2])
  {
    this->GridColor[0] = color[0];
    this->GridColor[1] = color[1];
    this->GridColor[2] = color[2];
    this->GridConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowGrid(bool show)
{
  if (this->GridVisible != show)
  {
    this->GridVisible = show;
    this->RenderPassesConfigured = false;
    this->GridConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureGridUsingCurrentActors()
{
  // Configure grid using visible prop bounds and actors
  // Also initialize GridInfo
  bool show = this->GridVisible;
  if (show)
  {
    double bounds[6];
    this->ComputeVisiblePropBounds(bounds);

    vtkBoundingBox bbox(bounds);

    if (!bbox.IsValid())
    {
      show = false;
    }
    else
    {
      double diag = bbox.GetDiagonalLength();

      double tmpUnitSquare;
      if (this->GridUnitSquare.has_value())
      {
        tmpUnitSquare = this->GridUnitSquare.value();
      }
      else
      {
        tmpUnitSquare = pow(10.0, round(log10(diag * 0.1)));
      }

      double gridPos[3] = { 0, 0, 0 };
      if (this->GridAbsolute)
      {
        for (int i = 0; i < 3; i++)
        {
          gridPos[i] = this->UpVector[i] ? 0 : 0.5 * (bounds[2 * i] + bounds[2 * i + 1]);
        }
      }
      else
      {
        for (int i = 0; i < 3; i++)
        {
          // a small margin is added to the size to avoid z-fighting if large translucent
          // triangles are exactly aligned with the grid bounds
          constexpr double margin = 1.0001;
          double size = margin * (bounds[2 * i + 1] - bounds[2 * i]);
          gridPos[i] = 0.5 * (bounds[2 * i] + bounds[2 * i + 1] - this->UpVector[i] * size);
        }
      }

      std::stringstream stream;
      stream << "Using grid unit square size = " << tmpUnitSquare << "\n"
             << "Grid origin set to [" << gridPos[0] << ", " << gridPos[1] << ", " << gridPos[2]
             << "]\n";
      this->GridInfo = stream.str();

      vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
      gridMapper->SetFadeDistance(diag);
      gridMapper->SetUnitSquare(tmpUnitSquare);
      gridMapper->SetSubdivisions(this->GridSubdivisions);
      gridMapper->SetUpIndex(this->UpIndex);
      if (this->GridAbsolute)
        gridMapper->SetOriginOffset(-gridPos[0], -gridPos[1], -gridPos[2]);

      this->GridActor->GetProperty()->SetColor(this->GridColor);
      this->GridActor->ForceTranslucentOn();
      this->GridActor->SetPosition(gridPos);
      this->GridActor->SetMapper(gridMapper);
      this->GridActor->UseBoundsOff();
      this->GridActor->PickableOff();
      this->GridConfigured = true;
    }
  }

  this->GridActor->SetVisibility(show);
  this->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetHDRIFile(const std::optional<std::string>& hdriFile)
{
  // Check HDRI is different than current one
  std::optional<std::string> collapsedHdriFile;
  if (hdriFile.has_value())
  {
    collapsedHdriFile = vtksys::SystemTools::CollapseFullPath(hdriFile.value());
  }

  if (this->HDRIFile != collapsedHdriFile)
  {
    this->HDRIFile = collapsedHdriFile;

    this->TextActorsConfigured = false;
    this->RenderPassesConfigured = false;

    this->HasValidHDRIReader = false;
    this->HasValidHDRIHash = false;
    this->HasValidHDRITexture = false;
    this->HasValidHDRISH = false;
    this->HasValidHDRISpec = false;

    this->HDRIReaderConfigured = false;
    this->HDRIHashConfigured = false;
    this->HDRITextureConfigured = false;
    this->HDRISphericalHarmonicsConfigured = false;
    this->HDRISpecularConfigured = false;
    this->HDRISkyboxConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseImageBasedLighting(bool use)
{
  if (use != this->GetUseImageBasedLighting())
  {
    this->Superclass::SetUseImageBasedLighting(use);

    this->HDRIReaderConfigured = false;
    this->HDRIHashConfigured = false;
    this->HDRITextureConfigured = false;
    this->HDRILUTConfigured = false;
    this->HDRISphericalHarmonicsConfigured = false;
    this->HDRISpecularConfigured = false;

    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetCachePath(const std::string& cachePath)
{
  if (this->CachePath != cachePath)
  {
    this->CachePath = cachePath;
    this->TextActorsConfigured = false;
    this->RenderPassesConfigured = false;

    this->HasValidHDRILUT = false;
    this->HasValidHDRISH = false;
    this->HasValidHDRISpec = false;

    this->HDRILUTConfigured = false;
    this->HDRISphericalHarmonicsConfigured = false;
    this->HDRISpecularConfigured = false;

    if (this->HasValidHDRIHash)
    {
      this->CreateCacheDirectory();
    }
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::CheckForSHCache(std::string& path)
{
  assert(this->HasValidHDRIHash);
  path = this->CachePath + "/" + this->HDRIHash + "/sh.vtt";
  return vtksys::SystemTools::FileExists(path, true);
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::CheckForSpecCache(std::string& path)
{
  assert(this->HasValidHDRIHash);
  path = this->CachePath + "/" + this->HDRIHash + "/specular.vtm";
  return vtksys::SystemTools::FileExists(path, true);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRI()
{
  if (!this->HDRIReaderConfigured)
  {
    this->ConfigureHDRIReader();
  }

  if (!this->HDRIHashConfigured)
  {
    this->ConfigureHDRIHash();
  }

  if (!this->HDRITextureConfigured)
  {
    this->ConfigureHDRITexture();
  }

  if (!this->HDRILUTConfigured)
  {
    this->ConfigureHDRILUT();
  }

  if (!this->HDRISphericalHarmonicsConfigured)
  {
    this->ConfigureHDRISphericalHarmonics();
  }

  if (!this->HDRISpecularConfigured)
  {
    this->ConfigureHDRISpecular();
  }

  if (!this->HDRISkyboxConfigured)
  {
    this->ConfigureHDRISkybox();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRIReader()
{
  if (!this->HasValidHDRIReader && (this->HDRISkyboxVisible || this->GetUseImageBasedLighting()))
  {
    this->UseDefaultHDRI = false;
    this->HDRIReader = nullptr;
    if (this->HDRIFile.has_value())
    {
      if (!vtksys::SystemTools::FileExists(this->HDRIFile.value(), true))
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, std::string("HDRI file does not exist ") + this->HDRIFile.value());
      }
      else
      {
        this->HDRIReader = vtkSmartPointer<vtkImageReader2>::Take(
          vtkImageReader2Factory::CreateImageReader2(this->HDRIFile.value().c_str()));
        if (this->HDRIReader)
        {
          this->HDRIReader->SetFileName(this->HDRIFile.value().c_str());
        }
        else
        {
          F3DLog::Print(F3DLog::Severity::Warning,
            std::string("Cannot open HDRI file ") + this->HDRIFile.value() +
              std::string(". Using default HDRI"));
        }
      }
    }

    if (!this->HDRIReader)
    {
      // No valid HDRI file have been provided, read the default HDRI
      // TODO add support for memory buffer in the vtkHDRReader in VTK
      // https://github.com/f3d-app/f3d/issues/935
      this->HDRIReader = vtkSmartPointer<vtkPNGReader>::New();
      this->HDRIReader->SetMemoryBuffer(F3DDefaultHDRI);
      this->HDRIReader->SetMemoryBufferLength(sizeof(F3DDefaultHDRI));
      this->UseDefaultHDRI = true;
    }
    this->HasValidHDRIReader = true;
  }
  this->HDRIReaderConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRIHash()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
  if (!this->HasValidHDRIHash && this->GetUseImageBasedLighting() && this->HasValidHDRIReader)
  {
    if (this->UseDefaultHDRI)
    {
      this->HDRIHash = "default";
    }
    else
    {
      // Compute HDRI MD5, here we know the HDRIFile has a value
      this->HDRIHash = ::ComputeFileHash(this->HDRIFile.value());
    }
    this->HasValidHDRIHash = true;
    this->CreateCacheDirectory();
  }
#endif
  this->HDRIHashConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRITexture()
{
  if (!this->HasValidHDRITexture)
  {
    bool needHDRITexture = this->HDRISkyboxVisible || this->GetUseImageBasedLighting();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
    if (this->HasValidHDRIHash)
    {
      std::string dummy;
      needHDRITexture = this->HDRISkyboxVisible ||
        (this->GetUseImageBasedLighting() &&
          (!this->CheckForSHCache(dummy) || !this->CheckForSpecCache(dummy) ||
            this->UseRaytracing));
    }
#endif

    if (needHDRITexture)
    {
      assert(this->HasValidHDRIReader);
      this->HDRIReader->Update();

      this->HDRITexture = vtkSmartPointer<vtkTexture>::New();
      this->HDRITexture->SetColorModeToDirectScalars();
      this->HDRITexture->MipmapOn();
      this->HDRITexture->InterpolateOn();
      this->HDRITexture->SetInputConnection(this->HDRIReader->GetOutputPort());

      // 8-bit textures are usually gamma-corrected
      if (this->HDRIReader->GetOutput() &&
        this->HDRIReader->GetOutput()->GetScalarType() == VTK_UNSIGNED_CHAR)
      {
        this->HDRITexture->UseSRGBColorSpaceOn();
      }
      this->HasValidHDRITexture = true;
    }
    else
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20230902)
      // IBL without textures has been added in VTK in
      // https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10454
      this->HDRITexture = nullptr;
#else
      vtkNew<vtkImageData> img;
      this->HDRITexture = vtkSmartPointer<vtkTexture>::New();
      this->HDRITexture->SetInputData(img);
#endif
      this->HasValidHDRITexture = false;
    }
  }

  if (this->GetUseImageBasedLighting())
  {
    this->SetEnvironmentTexture(this->HDRITexture);

    // No cache support before 20221220
    // IBL without textures has been added in VTK in
    // https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10454
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220) &&                                     \
  VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20230902)
    if (this->SphericalHarmonics)
    {
      this->SphericalHarmonics->Modified();
    }
#endif
  }
  else
  {
    this->SetEnvironmentTexture(nullptr);
  }

  this->HDRITextureConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRILUT()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
  if (this->GetUseImageBasedLighting() && !this->HasValidHDRILUT)
  {
    vtkF3DCachedLUTTexture* lut = vtkF3DCachedLUTTexture::SafeDownCast(this->EnvMapLookupTable);
    assert(lut);

    // Check LUT cache
    std::string lutCachePath = this->CachePath + "/lut.vti";
    bool lutCacheExists = vtksys::SystemTools::FileExists(lutCachePath, true);
    if (lutCacheExists)
    {
      lut->SetFileName(lutCachePath.c_str());
      lut->UseCacheOn();
    }
    else
    {
      if (!lut->GetTextureObject() || !this->HasValidHDRILUT)
      {
        lut->UseCacheOff();
        lut->Load(this);
        lut->PostRender(this);
      }
      assert(lut->GetTextureObject());

#ifndef __EMSCRIPTEN__
      vtkSmartPointer<vtkImageData> img = ::SaveTextureToImage(
        lut->GetTextureObject(), GL_TEXTURE_2D, 0, lut->GetLUTSize(), VTK_UNSIGNED_SHORT);
      assert(img);

      vtkNew<vtkXMLImageDataWriter> writer;
      writer->SetFileName(lutCachePath.c_str());
      writer->SetInputData(img);
      writer->Write();
#endif
    }
    this->HasValidHDRILUT = true;
  }
#endif
  this->HDRILUTConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRISphericalHarmonics()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
  if (this->GetUseImageBasedLighting() && !this->HasValidHDRISH)
  {
    // Check spherical harmonics cache
    std::string shCachePath;
    if (this->CheckForSHCache(shCachePath))
    {
      vtkNew<vtkXMLTableReader> reader;
      reader->SetFileName(shCachePath.c_str());
      reader->Update();

      this->SphericalHarmonics = vtkFloatArray::SafeDownCast(reader->GetOutput()->GetColumn(0));
    }
    else
    {
      if (!this->SphericalHarmonics ||
        this->HDRITexture->GetInput()->GetMTime() > this->SphericalHarmonics->GetMTime() ||
        !this->HasValidHDRISH)
      {
        vtkNew<vtkSphericalHarmonics> sh;
        sh->SetInputData(this->HDRITexture->GetInput());
        sh->Update();
        this->SphericalHarmonics = vtkFloatArray::SafeDownCast(
          vtkTable::SafeDownCast(sh->GetOutputDataObject(0))->GetColumn(0));
      }

#ifndef __EMSCRIPTEN__
      // Create spherical harmonics cache file
      vtkNew<vtkTable> table;
      table->AddColumn(this->SphericalHarmonics);

      vtkNew<vtkXMLTableWriter> writer;
      writer->SetInputData(table);
      writer->SetFileName(shCachePath.c_str());
      writer->Write();
#endif
    }
    this->HasValidHDRISH = true;
  }
#endif
  this->HDRISphericalHarmonicsConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRISpecular()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
  if (this->GetUseImageBasedLighting() && !this->HasValidHDRISpec)
  {
    vtkF3DCachedSpecularTexture* spec =
      vtkF3DCachedSpecularTexture::SafeDownCast(this->EnvMapPrefiltered);
    assert(spec);

    // Check specular cache
    std::string specCachePath;
    if (this->CheckForSpecCache(specCachePath))
    {
      spec->SetFileName(specCachePath.c_str());
      spec->UseCacheOn();
    }
    else
    {
      if (!spec->GetTextureObject() || !this->HasValidHDRISpec)
      {
        spec->UseCacheOff();
        spec->Load(this);
        spec->PostRender(this);
      }
      assert(spec->GetTextureObject());

#ifndef __EMSCRIPTEN__
      unsigned int nbLevels = spec->GetPrefilterLevels();
      unsigned int size = spec->GetPrefilterSize();

      vtkNew<vtkMultiBlockDataSet> mb;
      mb->SetNumberOfBlocks(nbLevels);

      for (unsigned int i = 0; i < nbLevels; i++)
      {
        vtkSmartPointer<vtkImageData> img = ::SaveTextureToImage(
          spec->GetTextureObject(), GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, size >> i, VTK_FLOAT);
        assert(img);
        mb->SetBlock(i, img);
      }

      vtkNew<vtkXMLMultiBlockDataWriter> writer;
      writer->SetCompressorTypeToNone();
      writer->SetDataModeToAppended();
      writer->EncodeAppendedDataOff();
      writer->SetHeaderTypeToUInt64();
      writer->SetFileName(specCachePath.c_str());
      writer->SetInputData(mb);
      writer->Write();
#endif
    }
    this->HasValidHDRISpec = true;
  }
#endif

  this->HDRISpecularConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRISkybox()
{
  this->SkyboxActor->SetTexture(this->HDRITexture);
  this->SkyboxActor->SetVisibility(this->HDRISkyboxVisible);
  this->HDRISkyboxConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureTextActors()
{
  // Dynamic text color
  double textColor[3];
  if (this->IsBackgroundDark())
  {
    textColor[0] = textColor[1] = textColor[2] = 0.9;
  }
  else
  {
    textColor[0] = textColor[1] = textColor[2] = 0.2;
  }
  this->FilenameActor->GetTextProperty()->SetColor(textColor);
  this->MetaDataActor->GetTextProperty()->SetColor(textColor);
  this->TimerActor->GetTextProperty()->SetColor(textColor);
  this->CheatSheetActor->GetTextProperty()->SetColor(0.8, 0.8, 0.8);
  this->DropZoneActor->GetTextProperty()->SetColor(textColor);

  // Font
  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();
  this->DropZoneActor->GetTextProperty()->SetFontFamilyToCourier();
  if (this->FontFile.has_value())
  {
    std::string tmpFontFile = vtksys::SystemTools::CollapseFullPath(this->FontFile.value());
    if (vtksys::SystemTools::FileExists(tmpFontFile, true))
    {
      this->FilenameActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->FilenameActor->GetTextProperty()->SetFontFile(tmpFontFile.c_str());
      this->MetaDataActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->MetaDataActor->GetTextProperty()->SetFontFile(tmpFontFile.c_str());
      this->TimerActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->TimerActor->GetTextProperty()->SetFontFile(tmpFontFile.c_str());
      this->CheatSheetActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->CheatSheetActor->GetTextProperty()->SetFontFile(tmpFontFile.c_str());
      this->DropZoneActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->DropZoneActor->GetTextProperty()->SetFontFile(tmpFontFile.c_str());
    }
    else
    {
      F3DLog::Print(
        F3DLog::Severity::Warning, std::string("Cannot find \"") + tmpFontFile + "\" font file.");
    }
  }

  this->TextActorsConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetLineWidth(const std::optional<double>& lineWidth)
{
  if (this->LineWidth != lineWidth)
  {
    this->LineWidth = lineWidth;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetPointSize(const std::optional<double>& pointSize)
{
  if (this->PointSize != pointSize)
  {
    this->PointSize = pointSize;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetFontFile(const std::optional<std::string>& fontFile)
{
  if (this->FontFile != fontFile)
  {
    this->FontFile = fontFile;
    this->TextActorsConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetBackground(const double* color)
{
  this->Superclass::SetBackground(color);
  this->TextActorsConfigured = false;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetLightIntensity(const double intensityFactor)
{
  if (this->LightIntensity != intensityFactor)
  {
    this->LightIntensity = intensityFactor;
    this->LightIntensitiesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetFilenameInfo(const std::string& info)
{
  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, info.c_str());
  this->RenderPassesConfigured = false;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetDropZoneInfo(const std::string& info)
{
  this->DropZoneActor->SetDropText(info);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseDepthPeelingPass(bool use)
{
  if (this->UseDepthPeelingPass != use)
  {
    this->UseDepthPeelingPass = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseBlurBackground(bool use)
{
  if (this->UseBlurBackground != use)
  {
    this->UseBlurBackground = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetBackfaceType(const std::optional<std::string>& backfaceType)
{
  if (this->BackfaceType != backfaceType)
  {
    this->BackfaceType = backfaceType;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetBlurCircleOfConfusionRadius(double radius)
{
  if (this->CircleOfConfusionRadius != radius)
  {
    this->CircleOfConfusionRadius = radius;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseSSAOPass(bool use)
{
  if (this->UseSSAOPass != use)
  {
    this->UseSSAOPass = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetFinalShader(const std::optional<std::string>& finalShader)
{
  if (this->FinalShader != finalShader)
  {
    this->FinalShader = finalShader;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseFXAAPass(bool use)
{
  if (this->UseFXAAPass != use)
  {
    this->UseFXAAPass = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseToneMappingPass(bool use)
{
  if (this->UseToneMappingPass != use)
  {
    this->UseToneMappingPass = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracing(bool use)
{
  if (this->UseRaytracing != use)
  {
    this->UseRaytracing = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetRaytracingSamples(int samples)
{
  if (this->RaytracingSamples != samples)
  {
    this->RaytracingSamples = samples;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracingDenoiser(bool use)
{
  if (this->UseRaytracingDenoiser != use)
  {
    this->UseRaytracingDenoiser = use;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowTimer(bool show)
{
  if (this->TimerVisible != show)
  {
    this->TimerVisible = show;
    this->TimerActor->SetVisibility(show);
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowFilename(bool show)
{
  if (this->FilenameVisible != show)
  {
    this->FilenameVisible = show;
    this->FilenameActor->SetVisibility(show);
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowMetaData(bool show)
{
  if (this->MetaDataVisible != show)
  {
    this->MetaDataVisible = show;
    this->MetaDataConfigured = false;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureMetaData()
{
  this->MetaDataActor->SetVisibility(this->MetaDataVisible);
  if (this->MetaDataVisible)
  {
    this->MetaDataActor->SetText(
      vtkCornerAnnotation::RightEdge, this->GenerateMetaDataDescription().c_str());
  }
  this->MetaDataConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowCheatSheet(bool show)
{
  if (this->CheatSheetVisible != show)
  {
    this->CheatSheetVisible = show;
    this->CheatSheetActor->SetVisibility(show);
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureCheatSheet(const std::string& info)
{
  if (this->CheatSheetVisible)
  {
    this->CheatSheetActor->SetText(vtkCornerAnnotation::LeftEdge, info.c_str());
    this->CheatSheetActor->RenderOpaqueGeometry(this);
    this->CheatSheetConfigured = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowDropZone(bool show)
{
  if (this->DropZoneVisible != show)
  {
    this->DropZoneVisible = show;
    this->DropZoneActor->SetVisibility(show);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowHDRISkybox(bool show)
{
  if (this->HDRISkyboxVisible != show)
  {
    this->HDRISkyboxVisible = show;

    this->HDRIReaderConfigured = false;
    this->HDRITextureConfigured = false;
    this->HDRISkyboxConfigured = false;
    this->RenderPassesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowEdge(const std::optional<bool>& show)
{
  if (this->EdgeVisible != show)
  {
    this->EdgeVisible = show;
    this->ActorsPropertiesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseOrthographicProjection(const std::optional<bool>& use)
{
  // if the internal state is already the same as the target state there's nothing to do
  if (this->UseOrthographicProjection != use)
  {
    this->UseOrthographicProjection = use;

    // XXX This could be done in UpdateActors for coherency
    if (this->UseOrthographicProjection.has_value())
    {
      vtkCamera* camera = GetActiveCamera();
      const double angle = vtkMath::RadiansFromDegrees(camera->GetViewAngle());
      const double* position = camera->GetPosition();
      const double* focal = camera->GetFocalPoint();

      if (this->UseOrthographicProjection.value())
      {
        const double distance = std::sqrt(vtkMath::Distance2BetweenPoints(position, focal));
        const double parallelScale = distance * tan(angle / 2);
        camera->SetParallelScale(parallelScale);
      }
      else
      {
        const double distance = camera->GetParallelScale() / tan(angle / 2);
        double direction[3];
        vtkMath::Subtract(position, focal, direction);
        vtkMath::Normalize(direction);
        vtkMath::MultiplyScalar(direction, distance);
        double newPosition[3];
        vtkMath::Add(focal, direction, newPosition);
        camera->SetPosition(newPosition);
      }
      camera->SetParallelProjection(this->UseOrthographicProjection.value());
      this->ResetCameraClippingRange();
    }
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseTrackball(bool use)
{
  if (this->UseTrackball != use)
  {
    this->UseTrackball = use;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateActors()
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
    this->ActorsPropertiesConfigured = false;
    this->ColoringConfigured = false;
  }
  this->ImporterTimeStamp = importerMTime;

  if (!this->ActorsPropertiesConfigured)
  {
    this->ConfigureActorsProperties();
  }

  if (!this->ColoringConfigured)
  {
    this->ConfigureColoring();
  }

  this->ConfigureHDRI();

  if (!this->MetaDataConfigured)
  {
    this->ConfigureMetaData();
  }

  if (!this->TextActorsConfigured)
  {
    this->ConfigureTextActors();
  }

  if (!this->RenderPassesConfigured)
  {
    this->ConfigureRenderPasses();
  }

  // Grid need all actors setup to be configured correctly
  if (!this->GridConfigured)
  {
    this->ConfigureGridUsingCurrentActors();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Render()
{
  if (!this->TimerVisible)
  {
    this->Superclass::Render();
    return;
  }

  auto cpuStart = std::chrono::high_resolution_clock::now();
  if (this->Timer == 0)
  {
    glGenQueries(1, &this->Timer);
  }

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
  glBeginQuery(GL_TIME_ELAPSED, this->Timer);
#endif

  this->TimerActor->RenderOpaqueGeometry(this); // update texture

  this->Superclass::Render();

  auto cpuElapsed = std::chrono::high_resolution_clock::now() - cpuStart;

  // Get CPU frame per seconds
  int fps = static_cast<int>(std::round(
    1.0 / (std::chrono::duration_cast<std::chrono::microseconds>(cpuElapsed).count() * 1e-6)));

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
  glEndQuery(GL_TIME_ELAPSED);
  GLint elapsed;
  glGetQueryObjectiv(this->Timer, GL_QUERY_RESULT, &elapsed);

  // Get min between CPU frame per seconds and GPU frame per seconds
  fps = std::min(fps, static_cast<int>(std::round(1.0 / (elapsed * 1e-9))));
#endif

  std::string str = std::to_string(fps);
  str += " fps";
  this->TimerActor->SetInput(str.c_str());
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ResetCameraClippingRange()
{
  const bool gridUseBounds = this->GridActor->GetUseBounds();
  this->GridActor->UseBoundsOn();
  this->Superclass::ResetCameraClippingRange();
  this->GridActor->SetUseBounds(gridUseBounds);
}

//----------------------------------------------------------------------------
int vtkF3DRenderer::UpdateLights()
{
  // Recover the number of lights that are on
  vtkLightCollection* lc = this->GetLights();
  vtkLight* light;
  int lightCount = 0;
  vtkCollectionSimpleIterator it;
  for (lc->InitTraversal(it); (light = lc->GetNextLight(it));)
  {
    if (light->GetSwitch())
    {
      lightCount++;
    }
  }

  // If no lights are turned on, add a light kit, even when using a HDRI
  if (lightCount == 0)
  {
    vtkNew<vtkLightKit> lightKit;
    lightKit->AddLightsToRenderer(this);
    this->LightIntensitiesConfigured = false;
  }

  // Update light shaders
  lightCount = this->Superclass::UpdateLights();

  if (!this->LightIntensitiesConfigured)
  {
    lc = this->GetLights();
    for (lc->InitTraversal(it); (light = lc->GetNextLight(it));)
    {
      double originalIntensity;
      if (this->OriginalLightIntensities.count(light))
      {
        originalIntensity = this->OriginalLightIntensities[light];
      }
      else
      {
        originalIntensity = light->GetIntensity();
        this->OriginalLightIntensities[light] = originalIntensity;
      }

      light->SetIntensity(originalIntensity * this->LightIntensity);
    }
    this->LightIntensitiesConfigured = true;
  }

  return lightCount;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsBackgroundDark()
{
  double luminance =
    0.299 * this->Background[0] + 0.587 * this->Background[1] + 0.114 * this->Background[2];
  return this->HDRISkyboxVisible ? true : luminance < 0.5;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::CreateCacheDirectory()
{
  assert(this->HasValidHDRIHash);

  // Cache folder for this HDRI
  std::string currentCachePath = this->CachePath + "/" + this->HDRIHash;

  // Create the folder if it does not exists
  vtksys::SystemTools::MakeDirectory(currentCachePath);
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::GenerateMetaDataDescription()
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
void vtkF3DRenderer::SetImporter(vtkF3DMetaImporter* importer)
{
  this->Importer = importer;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetRoughness(const std::optional<double>& roughness)
{
  if (this->Roughness != roughness)
  {
    this->Roughness = roughness;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetOpacity(const std::optional<double>& opacity)
{
  if (this->Opacity != opacity)
  {
    this->Opacity = opacity;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetMetallic(const std::optional<double>& metallic)
{
  if (this->Metallic != metallic)
  {
    this->Metallic = metallic;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetNormalScale(const std::optional<double>& normalScale)
{
  if (this->NormalScale != normalScale)
  {
    this->NormalScale = normalScale;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetSurfaceColor(const std::optional<std::vector<double>>& color)
{
  if (this->SurfaceColor != color)
  {
    this->SurfaceColor = color;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetEmissiveFactor(const std::optional<std::vector<double>>& factor)
{
  if (this->EmissiveFactor != factor)
  {
    this->EmissiveFactor = factor;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetTextureMatCap(const std::optional<std::string>& tex)
{
  if (this->TextureMatCap != tex)
  {
    this->TextureMatCap = tex;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetTextureBaseColor(const std::optional<std::string>& tex)
{
  if (this->TextureBaseColor != tex)
  {
    this->TextureBaseColor = tex;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetTextureMaterial(const std::optional<std::string>& tex)
{
  if (this->TextureMaterial != tex)
  {
    this->TextureMaterial = tex;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetTextureEmissive(const std::optional<std::string>& tex)
{
  if (this->TextureEmissive != tex)
  {
    this->TextureEmissive = tex;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetTextureNormal(const std::optional<std::string>& tex)
{
  if (this->TextureNormal != tex)
  {
    this->TextureNormal = tex;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureActorsProperties()
{
  assert(this->Importer);

  double* surfaceColor = nullptr;
  if (this->SurfaceColor.has_value())
  {
    if (this->SurfaceColor.value().size() != 3)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Invalid surface color provided, not applying"));
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
        std::string("Invalid emissive factor provided, not applying"));
    }
    else
    {
      emissiveFactor = this->EmissiveFactor.value().data();
    }
  }

  bool setBackfaceCulling = false;
  bool backfaceCulling = true;
  if (this->BackfaceType.has_value())
  {
    setBackfaceCulling = true;
    if (this->BackfaceType.value() == "visible")
    {
      backfaceCulling = false;
    }
    else if (this->BackfaceType.value() == "hidden")
    {
      backfaceCulling = true;
    }
    else
    {
      setBackfaceCulling = false;
      F3DLog::Print(F3DLog::Severity::Warning, this->BackfaceType.value() + " is not a valid backface type, assuming it is not set");
    }
  }

  for ([[maybe_unused]] const auto& [actor, mapper, originalActor] : this->Importer->GetColoringActorsAndMappers())
  {
    if (this->EdgeVisible.has_value())
    {
      actor->GetProperty()->SetEdgeVisibility(this->EdgeVisible.value());
      originalActor->GetProperty()->SetEdgeVisibility(this->EdgeVisible.value());
    }

    if (this->LineWidth.has_value())
    {
      actor->GetProperty()->SetLineWidth(this->LineWidth.value());
      originalActor->GetProperty()->SetLineWidth(this->LineWidth.value());
    }

    if (this->PointSize.has_value())
    {
      actor->GetProperty()->SetPointSize(this->PointSize.value());
      originalActor->GetProperty()->SetPointSize(this->PointSize.value());
    }

    if (setBackfaceCulling)
    {
      actor->GetProperty()->SetBackfaceCulling(backfaceCulling);
      originalActor->GetProperty()->SetBackfaceCulling(backfaceCulling);
    }

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

  this->ActorsPropertiesConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetPointSpritesProperties(SplatType type, double pointSpritesSize)
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
void vtkF3DRenderer::ShowScalarBar(bool show)
{
  if (this->ScalarBarVisible != show)
  {
    this->ScalarBarVisible = show;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUsePointSprites(bool use)
{
  if (this->UsePointSprites != use)
  {
    this->UsePointSprites = use;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseVolume(bool use)
{
  if (this->UseVolume != use)
  {
    this->UseVolume = use;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseInverseOpacityFunction(bool use)
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
void vtkF3DRenderer::SetScalarBarRange(const std::optional<std::vector<double>>& range)
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
void vtkF3DRenderer::SetColormap(const std::vector<double>& colormap)
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
void vtkF3DRenderer::SetEnableColoring(bool enable)
{
  if (enable != this->EnableColoring)
  {
    this->EnableColoring = enable;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseCellColoring(bool useCell)
{
  if (useCell != this->UseCellColoring)
  {
    this->UseCellColoring = useCell;
    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetArrayNameForColoring(const std::optional<std::string>& arrayName)
{
  if (arrayName != this->ArrayNameForColoring)
  {
    this->ArrayNameForColoring = arrayName;
    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
std::optional<std::string> vtkF3DRenderer::GetArrayNameForColoring()
{
  return this->ArrayNameForColoring;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetComponentForColoring(int component)
{
  if (component != this->ComponentForColoring)
  {
    this->ComponentForColoring = component;
    this->ColorTransferFunctionConfigured = false;
    this->ColoringMappersConfigured = false;
    this->PointSpritesMappersConfigured = false;
    this->VolumePropsAndMappersConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->CheatSheetConfigured = false;
    this->ColoringConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureColoring()
{
  assert(this->Importer);

  // Recover coloring information and update handler
  bool enableColoring = this->EnableColoring || (!this->UseRaytracing && this->UseVolume);
  F3DColoringInfoHandler& coloringHandler = this->Importer->GetColoringInfoHandler();
  auto info = coloringHandler.SetCurrentColoring(enableColoring, this->UseCellColoring, this->ArrayNameForColoring, false);
  bool hasColoring = info.has_value();
  if (hasColoring && !this->ColorTransferFunctionConfigured)
  {
    this->ConfigureRangeAndCTFForColoring(info.value());
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
          visible = vtkF3DRenderer::ConfigureMapperForColoring(mapper, info.value().Name,
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
          vtkF3DRenderer::ConfigureMapperForColoring(mapper, info.value().Name,
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
  bool volumeVisible = !this->UseRaytracing && this->UseVolume;
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
          visible = vtkF3DRenderer::ConfigureVolumeForColoring(mapper,
            prop, info.value().Name, this->ComponentForColoring,
            this->ColorTransferFunction, this->ColorRange, this->UseCellColoring,
            this->UseInverseOpacityFunction);
          if (!visible)
          {
            F3DLog::Print(
              F3DLog::Severity::Warning, "Cannot find the array \"" + info.value().Name + "\" to display volume with");
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
    vtkF3DRenderer::ConfigureScalarBarActorForColoring(
      this->ScalarBarActor, info.value().Name, this->ComponentForColoring, this->ColorTransferFunction);
    this->ScalarBarActorConfigured = true;
  }

  this->RenderPassesConfigured = false;
  this->ColoringConfigured = true;
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::GetColoringDescription()
{
  assert(this->Importer);

  std::stringstream stream;
  auto info = this->Importer->GetColoringInfoHandler().GetCurrentColoringInfo();
  if (info.has_value())
  {
    stream << "Coloring using " << (this->UseCellColoring ? "cell" : "point") << " array named "
           << info.value().Name << (this->EnableColoring ? ", " : " (forced), ")
           << vtkF3DRenderer::ComponentToString(this->ComponentForColoring);
  }
  else
  {
    stream << "Not coloring";
  }
  return stream.str();
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::ConfigureMapperForColoring(vtkPolyDataMapper* mapper, const std::string& name,
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
bool vtkF3DRenderer::ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper,
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
void vtkF3DRenderer::ConfigureScalarBarActorForColoring(
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
void vtkF3DRenderer::ConfigureRangeAndCTFForColoring(
  const F3DColoringInfoHandler::ColoringInfo& info)
{
  if (this->ComponentForColoring == -2)
  {
    return;
  }

  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      std::string("Invalid component index: ") + std::to_string(this->ComponentForColoring));
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
        std::string("Invalid scalar range provided, using automatic range"));
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
void vtkF3DRenderer::CycleFieldForColoring()
{
  // XXX: A generic approach will be better when adding categorical field data coloring
  this->SetUseCellColoring(!this->UseCellColoring);
  bool enableColoring = this->EnableColoring || (!this->UseRaytracing && this->UseVolume);
  F3DColoringInfoHandler& coloringHandler = this->Importer->GetColoringInfoHandler();
  auto info = coloringHandler.SetCurrentColoring(enableColoring, this->UseCellColoring, this->ArrayNameForColoring, true);
  if (!info.has_value())
  {
    // Cycle array if the current one is not valid
    this->CycleArrayForColoring();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::CycleArrayForColoring()
{
  assert(this->Importer);
  this->Importer->GetColoringInfoHandler().CycleColoringArray(!this->UseVolume); //TODO check this cond
  auto info = this->Importer->GetColoringInfoHandler().GetCurrentColoringInfo();
  bool enable = info.has_value();

  this->SetEnableColoring(enable);
  if (this->EnableColoring)
  {
    this->SetArrayNameForColoring(info.value().Name);
    if (this->ComponentForColoring >= info.value().MaximumNumberOfComponents)
    {
      // Cycle component if the current one is not valid
      this->CycleComponentForColoring();
    }
  }
  else
  {
    this->SetArrayNameForColoring(std::nullopt);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::CycleComponentForColoring()
{
  assert(this->Importer);

  auto info = this->Importer->GetColoringInfoHandler().GetCurrentColoringInfo();
  if (!info.has_value())
  {
    return;
  }

  // -2 -1 0 1 2 3 4
  this->SetComponentForColoring(
    (this->ComponentForColoring + 3) % (info.value().MaximumNumberOfComponents + 2) - 2);
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::ComponentToString(int component)
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
    auto info = this->Importer->GetColoringInfoHandler().GetCurrentColoringInfo();
    if (!info.has_value())
    {
      return "";
    }
    if (component >= info.value().MaximumNumberOfComponents)
    {
      return "";
    }

    std::string componentName;
    if (component < static_cast<int>(info.value().ComponentNames.size()))
    {
      componentName = info.value().ComponentNames[component];
    }
    if (componentName.empty())
    {
      componentName = "Component #";
      componentName += std::to_string(component);
    }
    return componentName;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::CheatSheetNeedsUpdate() const
{
  return this->CheatSheetVisible && !this->CheatSheetConfigured;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetCheatSheetConfigured(bool flag)
{
  this->CheatSheetConfigured = flag;
}
