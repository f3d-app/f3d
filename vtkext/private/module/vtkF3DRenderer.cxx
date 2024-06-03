#include "vtkF3DRenderer.h"

#include "F3DDefaultHDRI.h"
#include "F3DLog.h"
#include "vtkF3DCachedLUTTexture.h"
#include "vtkF3DCachedSpecularTexture.h"
#include "vtkF3DConfigure.h"
#include "vtkF3DDropZoneActor.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderPass.h"

#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
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
#include <vtkPixelBufferObject.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkSkybox.h>
#include <vtkTable.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextureObject.h>
#include <vtkToneMappingPass.h>
#include <vtkVersion.h>
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

#include <vtk_glew.h>

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
}

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
  // First version of VTK including the version check (and the feature used)
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200527)
  this->SkyboxActor->GammaCorrectOn();
#endif

  this->FilenameActor->VisibilityOff();
  this->MetaDataActor->VisibilityOff();
  this->TimerActor->VisibilityOff();
  this->CheatSheetActor->VisibilityOff();
  this->DropZoneActor->VisibilityOff();
  this->SkyboxActor->VisibilityOff();
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
void vtkF3DRenderer::Initialize(const std::string& up)
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

  this->AnimationNameInfo = "";
  this->GridInfo = "";

  // Importer rely on the Environment being set, so this is needed in the initialization
  const std::regex re("([-+]?)([XYZ])", std::regex_constants::icase);
  std::smatch match;
  if (std::regex_match(up, match, re))
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
    F3DLog::Print(F3DLog::Severity::Warning, up + " is not a valid up direction");
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
    toneP->SetToneMappingType(vtkToneMappingPass::GenericFilmic);
    toneP->SetGenericFilmicDefaultPresets();
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

  this->SetPass(renderingPass);

#if F3D_MODULE_RAYTRACING
  vtkOSPRayRendererNode::SetRendererType("pathtracer", this);
  vtkOSPRayRendererNode::SetSamplesPerPixel(this->RaytracingSamples, this);
  vtkOSPRayRendererNode::SetEnableDenoiser(this->UseRaytracingDenoiser, this);
  vtkOSPRayRendererNode::SetDenoiserThreshold(0, this);

// complete SetBackgroundMode needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7341
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210123)
  vtkOSPRayRendererNode::BackgroundMode mode = vtkOSPRayRendererNode::Backplate;
#else
  int mode = 1;
#endif
  if (this->GetUseImageBasedLighting())
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210123)
    mode = vtkOSPRayRendererNode::Both;
#else
    mode = 3;
#endif
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
      if (this->RenderWindow->GetInteractor())
      {
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
      else
      {
        F3DLog::Print(F3DLog::Severity::Error, "Axis widget cannot be shown without an interactor");
      }
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
void vtkF3DRenderer::SetGridUnitSquare(double unitSquare)
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
      double tmpUnitSquare = this->GridUnitSquare;
      if (tmpUnitSquare <= 0)
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
             << "]\n\n";
      this->GridInfo = stream.str();

      vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
      gridMapper->SetFadeDistance(diag);
      gridMapper->SetUnitSquare(tmpUnitSquare);
      gridMapper->SetSubdivisions(this->GridSubdivisions);
      gridMapper->SetUpIndex(this->UpIndex);
      if (this->GridAbsolute)
        gridMapper->SetOriginOffset(-gridPos[0], -gridPos[1], -gridPos[2]);

      this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
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
void vtkF3DRenderer::SetHDRIFile(const std::string& hdriFile)
{
  // Check HDRI is different than current one
  std::string collapsedHdriFile;
  if (!hdriFile.empty())
  {
    collapsedHdriFile = vtksys::SystemTools::CollapseFullPath(hdriFile);
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
    if (!this->HDRIFile.empty())
    {
      if (!vtksys::SystemTools::FileExists(this->HDRIFile, true))
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, std::string("HDRI file does not exist ") + this->HDRIFile);
      }
      else
      {
        this->HDRIReader = vtkSmartPointer<vtkImageReader2>::Take(
          vtkImageReader2Factory::CreateImageReader2(this->HDRIFile.c_str()));
        if (this->HDRIReader)
        {
          this->HDRIReader->SetFileName(this->HDRIFile.c_str());
        }
        else
        {
          F3DLog::Print(F3DLog::Severity::Warning,
            std::string("Cannot open HDRI file ") + this->HDRIFile +
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
      // Compute HDRI MD5
      this->HDRIHash = ::ComputeFileHash(this->HDRIFile);
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
  if (!this->FontFile.empty())
  {
    std::string tmpFontFile = vtksys::SystemTools::CollapseFullPath(this->FontFile);
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
void vtkF3DRenderer::SetLineWidth(double lineWidth)
{
  if (this->LineWidth != lineWidth)
  {
    this->LineWidth = lineWidth;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetPointSize(double pointSize)
{
  if (this->PointSize != pointSize)
  {
    this->PointSize = pointSize;
    this->ActorsPropertiesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetFontFile(const std::string& fontFile)
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
void vtkF3DRenderer::ConfigureCheatSheet()
{
  if (this->CheatSheetVisible)
  {
    std::stringstream cheatSheetText;
    cheatSheetText << "\n";
    this->FillCheatSheetHotkeys(cheatSheetText);
    cheatSheetText << "\n   H  : Cheat sheet \n";
    cheatSheetText << "   ?  : Print scene descr to terminal\n";
    cheatSheetText << "  ESC : Quit \n";
    cheatSheetText << " SPACE: Play animation if any\n";
    cheatSheetText << " LEFT : Previous file \n";
    cheatSheetText << " RIGHT: Next file \n";
    cheatSheetText << "  UP  : Reload current file \n";
    cheatSheetText << " DOWN : Add files from dir of current file\n";
    cheatSheetText << "\n 1: Front View camera\n";
    cheatSheetText << " 3: Right View camera\n";
    cheatSheetText << " 5: Toggle Orthographic Projection "
                   << (this->UseOrthographicProjection ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " 7: Top View camera\n";
    cheatSheetText << " 9: Isometric View camera\n";
    cheatSheetText << " ENTER: Reset camera to initial parameters\n";
    cheatSheetText << " Drop  : Load dropped file, folder or HDRI\n";

    this->CheatSheetActor->SetText(vtkCornerAnnotation::LeftEdge, cheatSheetText.str().c_str());
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
void vtkF3DRenderer::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{

  cheatSheetText << " W: Cycle animation ["
                 << vtkF3DRenderer::ShortName(this->AnimationNameInfo, 22) << "]\n";
  cheatSheetText << " P: Translucency support " << (this->UseDepthPeelingPass ? "[ON]" : "[OFF]")
                 << "\n";
  cheatSheetText << " Q: Ambient occlusion " << (this->UseSSAOPass ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " A: Anti-aliasing " << (this->UseFXAAPass ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " T: Tone mapping " << (this->UseToneMappingPass ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " E: Edge visibility " << (this->EdgeVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " X: Axis " << (this->AxisVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " G: Grid " << (this->GridVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " N: File name " << (this->FilenameVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " M: Metadata " << (this->MetaDataVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " Z: FPS Timer " << (this->TimerVisible ? "[ON]" : "[OFF]") << "\n";
#if F3D_MODULE_RAYTRACING
  cheatSheetText << " R: Raytracing " << (this->UseRaytracing ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " D: Denoiser " << (this->UseRaytracingDenoiser ? "[ON]" : "[OFF]") << "\n";
#endif
  cheatSheetText << " U: Blur background " << (this->UseBlurBackground ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " K: Trackball interaction " << (this->UseTrackball ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " F: HDRI ambient lighting "
                 << (this->GetUseImageBasedLighting() ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " J: HDRI skybox " << (this->HDRISkyboxVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText.precision(2);
  cheatSheetText << std::fixed;
  cheatSheetText << " L: Light (increase, shift+L: decrease) [" << this->LightIntensity << "]"
                 << " \n";
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureActorsProperties()
{
  vtkActor* anActor;
  vtkActorCollection* ac = this->GetActors();
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
  {
    if (vtkSkybox::SafeDownCast(anActor) == nullptr)
    {
      anActor->GetProperty()->SetEdgeVisibility(this->EdgeVisible);
      anActor->GetProperty()->SetLineWidth(this->LineWidth);
      anActor->GetProperty()->SetPointSize(this->PointSize);
    }
  }
  this->ActorsPropertiesConfigured = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowEdge(bool show)
{
  if (this->EdgeVisible != show)
  {
    this->EdgeVisible = show;
    this->ActorsPropertiesConfigured = false;
    this->CheatSheetConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseOrthographicProjection(bool use)
{
  // if the internal state is already the same as the target state there's nothing to do
  if (this->UseOrthographicProjection == use)
  {
    return;
  }

  vtkCamera* camera = GetActiveCamera();
  const double angle = vtkMath::RadiansFromDegrees(camera->GetViewAngle());
  const double* position = camera->GetPosition();
  const double* focal = camera->GetFocalPoint();

  if (use)
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
  this->UseOrthographicProjection = use;
  camera->SetParallelProjection(use);
  this->ResetCameraClippingRange();
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

  if (!this->GridConfigured)
  {
    this->ConfigureGridUsingCurrentActors();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Render()
{
  if (!this->ActorsPropertiesConfigured)
  {
    this->ConfigureActorsProperties();
  }

  if (!this->CheatSheetConfigured)
  {
    this->ConfigureCheatSheet();
  }

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
void vtkF3DRenderer::SetAnimationnameInfo(const std::string& info)
{
  this->AnimationNameInfo = info;
  this->CheatSheetConfigured = false;
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::ShortName(const std::string& name, int maxChar)
{
  if (name.size() <= static_cast<size_t>(maxChar) || maxChar <= 3)
  {
    return name;
  }
  else
  {
    return name.substr(0, maxChar - 3) + "...";
  }
}
