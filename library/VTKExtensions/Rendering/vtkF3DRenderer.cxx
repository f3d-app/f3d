#include "vtkF3DRenderer.h"

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

vtkStandardNewMacro(vtkF3DRenderer);

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
  file.open(filepath, std::ios_base::binary);
  file.read(buffer.data(), length);

  vtksysMD5* md5 = vtksysMD5_New();
  vtksysMD5_Initialize(md5);
  vtksysMD5_Append(md5, reinterpret_cast<const unsigned char*>(buffer.data()), length);
  vtksysMD5_Finalize(md5, digest);
  vtksysMD5_DigestToHex(digest, md5Hash);
  vtksysMD5_Delete(md5);

  return md5Hash;
}

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
}

//----------------------------------------------------------------------------
vtkF3DRenderer::vtkF3DRenderer()
{
  this->Cullers->RemoveAllItems();
  this->AutomaticLightCreationOff();
  this->SetClippingRangeExpansion(0.99);

  // Init actors
  this->MetaDataActor->GetTextProperty()->SetFontSize(15);
  this->MetaDataActor->GetTextProperty()->SetOpacity(0.5);
  this->MetaDataActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->MetaDataActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->TimerActor->GetTextProperty()->SetFontSize(15);
  this->TimerActor->SetPosition(10, 10);
  this->TimerActor->SetInput("0 fps");

  this->CheatSheetActor->GetTextProperty()->SetFontSize(14);
  this->CheatSheetActor->GetTextProperty()->SetOpacity(1.0);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundOpacity(0.8);

  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();
  this->DropZoneActor->GetTextProperty()->SetFontFamilyToCourier();

  this->FilenameActor->VisibilityOff();
  this->MetaDataActor->VisibilityOff();
  this->TimerActor->VisibilityOff();
  this->CheatSheetActor->VisibilityOff();
  this->DropZoneActor->VisibilityOff();
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
  this->RemoveAllViewProps();
  this->RemoveAllLights();
  this->OriginalLightIntensities.clear();

  this->AddActor(this->FilenameActor);
  this->AddActor(this->GridActor);
  this->AddActor(this->TimerActor);
  this->AddActor(this->MetaDataActor);
  this->AddActor(this->DropZoneActor);
  this->AddActor(this->CheatSheetActor);

  this->GridConfigured = false;
  this->CheatSheetConfigured = false;
  this->ActorsPropertiesConfigured = false;
  this->RenderPassesConfigured = false;
  this->LightIntensitiesConfigured = false;
  this->TextActorsConfigured = false;
  this->MetaDataConfigured = false;
  this->HDRIConfigured = false;

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
    this->Skybox->SetFloorPlane(this->UpVector[0], this->UpVector[1], this->UpVector[2], 0.0);
    this->Skybox->SetFloorRight(front[0], front[1], front[2]);

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
std::string vtkF3DRenderer::GenerateMetaDataDescription()
{
  return " Unavailable\n";
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
  newPass->SetForceOpaqueBackground(this->HasHDRILighting);

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
  vtkOSPRayRendererNode::SetBackgroundMode(
    this->HasHDRILighting ? vtkOSPRayRendererNode::Environment : vtkOSPRayRendererNode::Backplate,
    this);
#else
  vtkOSPRayRendererNode::SetBackgroundMode(this->HasHDRILighting ? 2 : 1, this);
#endif

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
      if (!this->GridAbsolute)
      {
        for (int i = 0; i < 3; i++)
        {
          double size = bounds[2 * i + 1] - bounds[2 * i];
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

      this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
      this->GridActor->ForceTranslucentOn();
      this->GridActor->SetPosition(gridPos);
      this->GridActor->SetMapper(gridMapper);
      this->GridActor->UseBoundsOff();
      this->GridConfigured = true;
    }
  }

  this->GridActor->SetVisibility(show);
  this->ResetCameraClippingRange();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowHDRISkybox(bool show)
{
  if (this->HDRISkyboxVisible != show)
  {
    this->HDRISkyboxVisible = show;

    // TODO merely changing the Skybox visibility could be optimized
    // instead of configuring the HDRI lighting again
    this->HDRIConfigured = false;
    this->TextActorsConfigured = false;
    this->RenderPassesConfigured = false;
  }
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
    this->HDRIConfigured = false;
    this->TextActorsConfigured = false;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetCachePath(const std::string& cachePath)
{
  if (this->CachePath != cachePath)
  {
    this->CachePath = cachePath;
    this->HDRIConfigured = false;
    this->TextActorsConfigured = false;
    this->RenderPassesConfigured = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ConfigureHDRI()
{
  // Read HDRI when needed
  vtkNew<vtkTexture> hdriTexture;
  vtkSmartPointer<vtkImageReader2> reader;
  this->HasHDRILighting = false;
  if (!this->HDRIFile.empty())
  {
    if (!vtksys::SystemTools::FileExists(this->HDRIFile, true))
    {
      F3DLog::Print(
        F3DLog::Severity::Warning, std::string("HDRI file does not exist ") + this->HDRIFile);
    }
    else
    {
      reader = vtkSmartPointer<vtkImageReader2>::Take(
        vtkImageReader2Factory::CreateImageReader2(this->HDRIFile.c_str()));
      if (reader)
      {
        this->HasHDRILighting = true;
      }
      else
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, std::string("Cannot open HDRI file ") + this->HDRIFile);
      }
    }
  }

  // Dynamic HDRI
  if (this->HasHDRILighting)
  {

    bool needHDRIComputation = true;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
    // Check LUT cache
    std::string lutCachePath = this->CachePath + "/lut.vti";
    bool lutCacheExists = vtksys::SystemTools::FileExists(lutCachePath, true);
    if (lutCacheExists)
    {
      // TODO: this->EnvMapLookupTable should be a smart pointer in VTK
      if (this->EnvMapLookupTable)
      {
        this->EnvMapLookupTable->Delete();
      }

      vtkF3DCachedLUTTexture* lut = vtkF3DCachedLUTTexture::New();
      lut->SetFileName(lutCachePath.c_str());
      this->EnvMapLookupTable = lut;
    }

    // Compute HDRI MD5
    std::string hash = ::ComputeFileHash(this->HDRIFile);

    // Cache folder for this HDRI
    std::string currentCachePath = this->CachePath + "/" + hash;

    // Create the folder if it does not exists
    vtksys::SystemTools::MakeDirectory(currentCachePath);

    // Check spherical harmonics cache
    std::string shCachePath = this->CachePath + "/" + hash + "/sh.vtt";
    bool shCacheExists = vtksys::SystemTools::FileExists(shCachePath, true);
    if (shCacheExists)
    {
      vtkNew<vtkXMLTableReader> reader;
      reader->SetFileName(shCachePath.c_str());
      reader->Update();

      this->SphericalHarmonics = vtkFloatArray::SafeDownCast(reader->GetOutput()->GetColumn(0));
    }

    // Check specular cache
    std::string specCachePath = this->CachePath + "/" + hash + "/specular.vtm";
    bool specCacheExists = vtksys::SystemTools::FileExists(specCachePath, true);
    if (specCacheExists)
    {
      // TODO: this->EnvMapPrefiltered should be a smart pointer in VTK
      if (this->EnvMapPrefiltered)
      {
        this->EnvMapPrefiltered->Delete();
      }

      vtkF3DCachedSpecularTexture* spec = vtkF3DCachedSpecularTexture::New();
      spec->SetFileName(specCachePath.c_str());
      this->EnvMapPrefiltered = spec;
    }

    this->GetEnvMapPrefiltered()->HalfPrecisionOff();
    needHDRIComputation = !lutCacheExists || !shCacheExists || !specCacheExists;
#endif

    this->UseImageBasedLightingOn();

    if (needHDRIComputation || this->HDRISkyboxVisible)
    {
      // The actual env texture is needed, read it
      reader->SetFileName(this->HDRIFile.c_str());
      reader->Update();

      hdriTexture->SetColorModeToDirectScalars();
      hdriTexture->MipmapOn();
      hdriTexture->InterpolateOn();
      hdriTexture->SetInputConnection(reader->GetOutputPort());

      // 8-bit textures are usually gamma-corrected
      if (reader->GetOutput() && reader->GetOutput()->GetScalarType() == VTK_UNSIGNED_CHAR)
      {
        hdriTexture->UseSRGBColorSpaceOn();
      }
    }
    else
    {
      // TODO Improve vtkOpenGLRenderer to support not having a texture
      vtkNew<vtkImageData> img;
      hdriTexture->SetInputData(img);

      // The MTime must be more recent than the image one
      // Otherwise VTK triggers a new SH computation
      this->SphericalHarmonics->Modified();
    }
    this->SetEnvironmentTexture(hdriTexture);

    if (needHDRIComputation)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
      this->Render();

      // Create LUT cache file
      if (!lutCacheExists)
      {
        vtkPBRLUTTexture* lut = this->GetEnvMapLookupTable();
        assert(lut->GetTextureObject());

        vtkSmartPointer<vtkImageData> img = ::SaveTextureToImage(
          lut->GetTextureObject(), GL_TEXTURE_2D, 0, lut->GetLUTSize(), VTK_UNSIGNED_SHORT);
        assert(img);

        vtkNew<vtkXMLImageDataWriter> writer;
        writer->SetFileName(lutCachePath.c_str());
        writer->SetInputData(img);
        writer->Write();
      }

      // Create spherical harmonics cache file
      if (!shCacheExists)
      {
        vtkNew<vtkTable> table;
        table->AddColumn(this->SphericalHarmonics);

        vtkNew<vtkXMLTableWriter> writer;
        writer->SetInputData(table);
        writer->SetFileName(shCachePath.c_str());
        writer->Write();
      }

      // Create specular cache file
      if (!specCacheExists)
      {
        vtkPBRPrefilterTexture* spec = this->GetEnvMapPrefiltered();
        assert(spec->GetTextureObject());

        unsigned int nbLevels = spec->GetPrefilterLevels();
        unsigned int size = spec->GetPrefilterSize();

        vtkNew<vtkMultiBlockDataSet> mb;
        mb->SetNumberOfBlocks(6 * nbLevels);

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
        writer->SetFileName(specCachePath.c_str());
        writer->SetInputData(mb);
        writer->Write();
      }
#endif
    }

    if (this->HDRISkyboxVisible)
    {
      // Setup the OpenGL Skybox
      // TODO: Add support for visibility in vtkOpenGLSkybox
      this->AddActor(this->Skybox);
      this->Skybox->SetProjection(vtkSkybox::Sphere);
      this->Skybox->SetTexture(hdriTexture);

      // First version of VTK including the version check (and the feature used)
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200527)
      this->Skybox->GammaCorrectOn();
#endif
    }
    else
    {
      this->RemoveActor(this->Skybox);
    }
  }
  else
  {
    this->UseImageBasedLightingOff();
    this->SetEnvironmentTexture(nullptr);
    this->RemoveActor(this->Skybox);
  }
  this->HDRIConfigured = true;
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
void vtkF3DRenderer::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
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
  if (!this->HDRIConfigured)
  {
    this->ConfigureHDRI();
  }

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
int vtkF3DRenderer::UpdateLights()
{
  // Recover the number of light that are on
  int lightCount = this->Superclass::UpdateLights();

  // If no lights are turned on, add a light kit
  if (lightCount == 0 && !this->UseImageBasedLighting)
  {
    int nLights = this->GetLights()->GetNumberOfItems();
    vtkNew<vtkLightKit> lightKit;
    lightKit->AddLightsToRenderer(this);
    lightCount += this->GetLights()->GetNumberOfItems() - nLights;
    this->LightIntensitiesConfigured = false;
  }

  if (!this->LightIntensitiesConfigured)
  {
    vtkLightCollection* lc = this->GetLights();
    vtkLight* light;
    vtkCollectionSimpleIterator it;
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
  return this->HasHDRILighting ? true : luminance < 0.5;
}
