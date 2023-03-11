#include "vtkF3DRenderer.h"

#include "F3DLog.h"
#include "vtkF3DCachedLUTTexture.h"
#include "vtkF3DCachedSpecularTexture.h"
#include "vtkF3DConfigure.h"
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
#include <sstream>

vtkStandardNewMacro(vtkF3DRenderer);

namespace
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
//----------------------------------------------------------------------------
// Compute the MD5 hash of the scalar field of an image data
std::string ComputeImageHash(vtkImageData* image)
{
  unsigned char digest[16];
  char md5Hash[33];
  md5Hash[32] = '\0';

  unsigned char* content = reinterpret_cast<unsigned char*>(image->GetScalarPointer());
  int* dims = image->GetDimensions();
  int nbComp = image->GetNumberOfScalarComponents();
  int scalarSize = image->GetScalarSize();
  int size = nbComp * scalarSize * dims[0] * dims[1] * dims[2];

  vtksysMD5* md5 = vtksysMD5_New();
  vtksysMD5_Initialize(md5);
  vtksysMD5_Append(md5, content, size);
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

  // Init actors
  this->MetaDataActor->GetTextProperty()->SetFontSize(15);
  this->MetaDataActor->GetTextProperty()->SetOpacity(0.5);
  this->MetaDataActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->MetaDataActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->TimerActor->GetTextProperty()->SetFontSize(15);
  this->TimerActor->SetPosition(10, 10);
  this->TimerActor->SetInput("0 fps");

  this->CheatSheetActor->GetTextProperty()->SetFontSize(15);
  this->CheatSheetActor->GetTextProperty()->SetOpacity(0.5);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();

  this->FilenameActor->SetVisibility(false);
  this->MetaDataActor->SetVisibility(false);
  this->TimerActor->SetVisibility(false);
  this->CheatSheetActor->SetVisibility(false);
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
void vtkF3DRenderer::Initialize(const std::string& fileInfo, const std::string& up)
{
  this->RemoveAllViewProps();
  this->RemoveAllLights();
  this->OriginalLightIntensities.clear();

  this->AddActor(this->FilenameActor);
  this->AddActor(this->GridActor);
  this->AddActor(this->TimerActor);
  this->AddActor(this->MetaDataActor);
  this->AddActor(this->CheatSheetActor);

  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, fileInfo.c_str());
  this->GridInitialized = false;
  this->GridInfo = "";

  // Importer rely on the Environment being set, so this is needed in the initialization
  if (up.size() == 2)
  {
    char sign = up[0];
    char axis = std::toupper(up[1]);
    if ((sign == '-' || sign == '+') && (axis >= 'X' && axis <= 'Z'))
    {
      this->UpIndex = axis - 'X';
      std::fill(this->UpVector, this->UpVector + 3, 0);
      this->UpVector[this->UpIndex] = (sign == '+') ? 1.0 : -1.0;

      std::fill(this->RightVector, this->RightVector + 3, 0);
      this->RightVector[this->UpIndex == 0 ? 1 : 0] = 1.0;

      double pos[3];
      vtkMath::Cross(this->UpVector, this->RightVector, pos);
      vtkMath::MultiplyScalar(pos, -1.0);

      vtkCamera* cam = this->GetActiveCamera();
      cam->SetFocalPoint(0.0, 0.0, 0.0);
      cam->SetPosition(pos);
      cam->SetViewUp(this->UpVector);

      this->SetEnvironmentUp(this->UpVector);
      this->SetEnvironmentRight(this->RightVector);
    }
  }
}

//----------------------------------------------------------------------------
std::string vtkF3DRenderer::GenerateMetaDataDescription()
{
  return " Unavailable\n";
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetupRenderPasses()
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
  newPass->SetForceOpaqueBackground(this->HasHDRI);

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
    this->HasHDRI ? vtkOSPRayRendererNode::Environment : vtkOSPRayRendererNode::Backplate, this);
#else
  vtkOSPRayRendererNode::SetBackgroundMode(this->HasHDRI ? 2 : 1, this);
#endif

#else
  if (this->UseRaytracing || this->UseRaytracingDenoiser)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      "Raytracing options can't be used if F3D has not been built with raytracing");
  }
#endif
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
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowGrid(bool show, double unitSquare, int subdivisions)
{
  // Initialize grid using visible prop bounds
  // Also initialize GridInfo
  if (!this->GridInitialized)
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
      if (unitSquare <= 0)
      {
        unitSquare = pow(10.0, round(log10(diag * 0.1)));
      }

      double gridPos[3];
      for (int i = 0; i < 3; i++)
      {
        double size = bounds[2 * i + 1] - bounds[2 * i];
        gridPos[i] = 0.5 * (bounds[2 * i] + bounds[2 * i + 1] - this->UpVector[i] * size);
      }

      std::stringstream stream;
      stream << "Using grid unit square size = " << unitSquare << "\n"
             << "Grid origin set to [" << gridPos[0] << ", " << gridPos[1] << ", " << gridPos[2]
             << "]\n\n";
      this->GridInfo = stream.str();

      vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
      gridMapper->SetFadeDistance(diag);
      gridMapper->SetUnitSquare(unitSquare);
      gridMapper->SetSubdivisions(subdivisions);
      gridMapper->SetUpIndex(this->UpIndex);

      this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
      this->GridActor->ForceTranslucentOn();
      this->GridActor->SetPosition(gridPos);
      this->GridActor->SetMapper(gridMapper);
      this->GridActor->UseBoundsOff();
      this->GridActor->SetVisibility(false);
      this->SetClippingRangeExpansion(0);
      this->GridInitialized = true;
      this->GridVisible = false;
    }
  }

  // Actual grid visibility code
  if (this->GridVisible != show)
  {
    this->SetClippingRangeExpansion(show ? 0.99 : 0);
    this->GridVisible = show;
    this->GridActor->SetVisibility(show);
    this->ResetCameraClippingRange();

    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
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

    // Read HDRI when needed
    vtkNew<vtkTexture> hdriTexture;
    this->HasHDRI = false;
    if (!this->HDRIFile.empty())
    {
      this->HDRIFile = vtksys::SystemTools::CollapseFullPath(this->HDRIFile);
      if (!vtksys::SystemTools::FileExists(this->HDRIFile, true))
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, std::string("HDRI file does not exist ") + this->HDRIFile);
      }
      else
      {
        auto reader = vtkSmartPointer<vtkImageReader2>::Take(
          vtkImageReader2Factory::CreateImageReader2(this->HDRIFile.c_str()));
        if (reader)
        {
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

          this->HasHDRI = true;
        }
        else
        {
          F3DLog::Print(
            F3DLog::Severity::Warning, std::string("Cannot open HDRI file ") + this->HDRIFile);
        }
      }
    }

    // Dynamic HDRI
    if (this->HasHDRI)
    {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
      // Check LUT cache
      std::string lutCachePath = this->CachePath + "/lut.vti";
      bool lutCacheExists = vtksys::SystemTools::FileExists(lutCachePath, true);
      if (lutCacheExists)
      {
        vtkF3DCachedLUTTexture* lut = vtkF3DCachedLUTTexture::New();
        lut->SetFileName(lutCachePath.c_str());
        this->EnvMapLookupTable = lut;
      }

      // Compute HDRI MD5
      std::string hash = ::ComputeImageHash(hdriTexture->GetInput());

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
        vtkF3DCachedSpecularTexture* spec = vtkF3DCachedSpecularTexture::New();
        spec->SetFileName(specCachePath.c_str());
        this->EnvMapPrefiltered = spec;
      }

      this->GetEnvMapPrefiltered()->HalfPrecisionOff();
#endif

      // HDRI OpenGL
      this->UseImageBasedLightingOn();
      this->SetEnvironmentTexture(hdriTexture);

      // Skybox OpenGL
      this->Skybox->SetProjection(vtkSkybox::Sphere);
      this->Skybox->SetTexture(hdriTexture);

      // First version of VTK including the version check (and the feature used)
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200527)
      this->Skybox->GammaCorrectOn();
#endif

      // skybox orientation
      double front[3];
      vtkMath::Cross(this->RightVector, this->UpVector, front);
      this->Skybox->SetFloorPlane(this->UpVector[0], this->UpVector[1], this->UpVector[2], 0.0);
      this->Skybox->SetFloorRight(front[0], front[1], front[2]);

      this->AddActor(this->Skybox);

      // build HDRI textures and spherical harmonics
      this->Render();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221220)
      // Create LUT cache file
      if (!lutCacheExists)
      {
        vtkPBRLUTTexture* lut = this->GetEnvMapLookupTable();

        vtkSmartPointer<vtkImageData> img = ::SaveTextureToImage(
          lut->GetTextureObject(), GL_TEXTURE_2D, 0, lut->GetLUTSize(), VTK_UNSIGNED_SHORT);

        if (img)
        {
          vtkNew<vtkXMLImageDataWriter> writer;
          writer->SetFileName(lutCachePath.c_str());
          writer->SetInputData(img);
          writer->Write();
        }
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

        unsigned int nbLevels = spec->GetPrefilterLevels();
        unsigned int size = spec->GetPrefilterSize();

        vtkNew<vtkMultiBlockDataSet> mb;
        mb->SetNumberOfBlocks(6 * nbLevels);

        for (unsigned int i = 0; i < nbLevels; i++)
        {
          vtkSmartPointer<vtkImageData> img = ::SaveTextureToImage(
            spec->GetTextureObject(), GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, size >> i, VTK_FLOAT);

          mb->SetBlock(i, img);
        }

        vtkNew<vtkXMLMultiBlockDataWriter> writer;
        writer->SetFileName(specCachePath.c_str());
        writer->SetInputData(mb);
        writer->Write();
      }
#endif
    }
    else
    {
      this->UseImageBasedLightingOff();
      this->SetEnvironmentTexture(nullptr);
      this->RemoveActor(this->Skybox);
    }
    this->UpdateTextColor();
  }
  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateTextColor()
{
  // Dynamic text color
  double textColor[3];
  if (this->IsBackgroundDark())
  {
    textColor[0] = textColor[1] = textColor[2] = 1.0;
  }
  else
  {
    textColor[0] = textColor[1] = textColor[2] = 0.0;
  }
  this->FilenameActor->GetTextProperty()->SetColor(textColor);
  this->MetaDataActor->GetTextProperty()->SetColor(textColor);
  this->TimerActor->GetTextProperty()->SetColor(textColor);
  this->CheatSheetActor->GetTextProperty()->SetColor(textColor);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetLineWidth(double lineWidth)
{
  vtkActor* anActor;
  vtkActorCollection* ac = this->GetActors();
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
  {
    if (vtkSkybox::SafeDownCast(anActor) == nullptr)
    {
      anActor->GetProperty()->SetLineWidth(lineWidth);
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetPointSize(double pointSize)
{
  vtkActor* anActor;
  vtkActorCollection* ac = this->GetActors();
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
  {
    if (vtkSkybox::SafeDownCast(anActor) == nullptr)
    {
      anActor->GetProperty()->SetPointSize(pointSize);
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetFontFile(const std::string& fontFile)
{
  // Dynamic font management
  if (this->FontFile != fontFile)
  {
    this->FontFile = fontFile;
    this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
    this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
    this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
    this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();
    if (!fontFile.empty())
    {
      std::string tmpFontFile = vtksys::SystemTools::CollapseFullPath(fontFile);
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
      }
      else
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, std::string("Cannot find \"") + tmpFontFile + "\" font file.");
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetBackground(const double* color)
{
  this->Superclass::SetBackground(color);
  this->UpdateTextColor();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetLightIntensity(const double intensityFactor)
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

    light->SetIntensity(originalIntensity * intensityFactor);
  }
  this->LightIntensity = intensityFactor;
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseDepthPeelingPass(bool use)
{
  if (this->UseDepthPeelingPass != use)
  {
    this->UseDepthPeelingPass = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseBlurBackground(bool use)
{
  if (this->UseBlurBackground != use)
  {
    this->UseBlurBackground = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}
//----------------------------------------------------------------------------
void vtkF3DRenderer::SetBlurCircleOfConfusionRadius(double radius)
{
  if (this->CircleOfConfusionRadius != radius)
  {
    this->CircleOfConfusionRadius = radius;
    this->SetupRenderPasses();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseSSAOPass(bool use)
{
  if (this->UseSSAOPass != use)
  {
    this->UseSSAOPass = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseFXAAPass(bool use)
{
  if (this->UseFXAAPass != use)
  {
    this->UseFXAAPass = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseToneMappingPass(bool use)
{
  if (this->UseToneMappingPass != use)
  {
    this->UseToneMappingPass = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracing(bool use)
{
  if (this->UseRaytracing != use)
  {
    this->UseRaytracing = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetRaytracingSamples(int samples)
{
  if (this->RaytracingSamples != samples)
  {
    this->RaytracingSamples = samples;
    this->SetupRenderPasses();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracingDenoiser(bool use)
{
  if (this->UseRaytracingDenoiser != use)
  {
    this->UseRaytracingDenoiser = use;
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowTimer(bool show)
{
  if (this->TimerVisible != show)
  {
    this->TimerVisible = show;
    this->TimerActor->SetVisibility(show);
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowFilename(bool show)
{
  if (this->FilenameVisible != show)
  {
    this->FilenameVisible = show;
    this->FilenameActor->SetVisibility(show);
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowMetaData(bool show)
{
  if (this->MetaDataVisible != show)
  {
    this->MetaDataVisible = show;
    this->MetaDataActor->SetVisibility(show);
    if (show)
    {
      // Update metadata info
      std::string MetaDataDesc = this->GenerateMetaDataDescription();
      this->MetaDataActor->SetText(vtkCornerAnnotation::RightEdge, MetaDataDesc.c_str());
    }

    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowCheatSheet(bool show)
{
  if (this->CheatSheetVisible != show)
  {
    this->CheatSheetVisible = show;
    this->CheatSheetActor->SetVisibility(show);
    this->SetupRenderPasses();
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::UpdateCheatSheet()
{
  if (this->CheatSheetVisible)
  {
    std::stringstream cheatSheetText;
    cheatSheetText << "\n";
    this->FillCheatSheetHotkeys(cheatSheetText);
    cheatSheetText << "\n   H  : Cheat sheet \n";
    cheatSheetText << "   ?  : Print scene descr to terminal\n";
    cheatSheetText << "  ESC : Quit \n";
    cheatSheetText << " ENTER: Reset camera to initial parameters\n";
    cheatSheetText << " SPACE: Play animation if any\n";
    cheatSheetText << " LEFT : Previous file \n";
    cheatSheetText << " RIGHT: Next file \n";
    cheatSheetText << "  UP  : Reload current file \n";

    this->CheatSheetActor->SetText(vtkCornerAnnotation::LeftEdge, cheatSheetText.str().c_str());
    this->CheatSheetActor->RenderOpaqueGeometry(this);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
  cheatSheetText << " P: Depth peeling " << (this->UseDepthPeelingPass ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " Q: SSAO " << (this->UseSSAOPass ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " A: FXAA " << (this->UseFXAAPass ? "[ON]" : "[OFF]") << "\n";
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
void vtkF3DRenderer::ShowEdge(bool show)
{
  if (this->EdgeVisible != show)
  {
    this->EdgeVisible = show;

    // Dynamic edges
    vtkActor* anActor;
    vtkActorCollection* ac = this->GetActors();
    vtkCollectionSimpleIterator ait;
    for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
    {
      if (vtkSkybox::SafeDownCast(anActor) == nullptr)
      {
        anActor->GetProperty()->SetEdgeVisibility(show);
      }
    }

    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseTrackball(bool use)
{
  if (this->UseTrackball != use)
  {
    this->UseTrackball = use;
    this->CheatSheetNeedUpdate = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Render()
{
  if (this->CheatSheetNeedUpdate)
  {
    this->UpdateCheatSheet();
    this->CheatSheetNeedUpdate = false;
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

#ifndef __ANDROID__
  glBeginQuery(GL_TIME_ELAPSED, this->Timer);
#endif

  this->TimerActor->RenderOpaqueGeometry(this); // update texture

  this->Superclass::Render();

  auto cpuElapsed = std::chrono::high_resolution_clock::now() - cpuStart;

  // Get CPU frame per seconds
  int fps = static_cast<int>(std::round(
    1.0 / (std::chrono::duration_cast<std::chrono::microseconds>(cpuElapsed).count() * 1e-6)));

#ifndef __ANDROID__
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
  int lightCount = this->Superclass::UpdateLights();
  if (lightCount == 0 && !this->UseImageBasedLighting)
  {
    vtkNew<vtkLightKit> lightKit;
    lightKit->AddLightsToRenderer(this);
    return this->GetLights()->GetNumberOfItems();
  }
  return lightCount;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsBackgroundDark()
{
  double luminance =
    0.299 * this->Background[0] + 0.587 * this->Background[1] + 0.114 * this->Background[2];
  return this->HasHDRI ? true : luminance < 0.5;
}
