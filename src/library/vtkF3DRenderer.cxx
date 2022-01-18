#include "vtkF3DRenderer.h"

#include "F3DConfig.h"
#include "F3DLog.h"
#include "f3d_options.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderPass.h"

#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCornerAnnotation.h>
#include <vtkCullerCollection.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLTexture.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkToneMappingPass.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#if F3D_MODULE_RAYTRACING
#include <vtkOSPRayRendererNode.h>
#endif

#include <vtk_glew.h>

#include <cctype>
#include <chrono>

vtkStandardNewMacro(vtkF3DRenderer);

//----------------------------------------------------------------------------
vtkF3DRenderer::vtkF3DRenderer()
{
  this->Cullers->RemoveAllItems();
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
void vtkF3DRenderer::Initialize(const f3d::options& options, const std::string& fileInfo)
{
  if (!this->RenderWindow)
  {
    F3DLog::Print(F3DLog::Severity::Error, "No render window linked");
    return;
  }

  this->RemoveAllViewProps();
  this->RemoveAllLights();

  options.get("grid", this->GridVisible);
  options.get("axis", this->AxisVisible);
  options.get("edges", this->EdgesVisible);
  options.get("fps", this->TimerVisible);
  options.get("filename", this->FilenameVisible);
  options.get("metadata", this->MetaDataVisible);
  options.get("raytracing", this->UseRaytracing);
  options.get("samples", this->RaytracingSamples);
  options.get("denoise", this->UseRaytracingDenoiser);
  options.get("depth-peeling", this->UseDepthPeeling);
  options.get("ssao", this->UseSSAOPass);
  options.get("fxaa", this->UseFXAAPass);
  options.get("tone-mapping", this->UseToneMappingPass);
  options.get("blur-background", this->UseBlurBackground);
  options.get("trackball", this->UseTrackball);
  options.get("hdri", this->HDRIFile);
  options.get("verbose", this->Verbose);

  if (!this->HDRIFile.empty() && !this->GetUseImageBasedLighting())
  {
    this->HDRIFile = vtksys::SystemTools::CollapseFullPath(this->HDRIFile);
    if (!vtksys::SystemTools::FileExists(this->HDRIFile, true))
    {
      F3DLog::Print(F3DLog::Severity::Warning, "HDRI file does not exist ", this->HDRIFile);
    }
    else
    {
      auto reader = vtkSmartPointer<vtkImageReader2>::Take(
        vtkImageReader2Factory::CreateImageReader2(this->HDRIFile.c_str()));
      if (reader)
      {
        reader->SetFileName(this->HDRIFile.c_str());
        reader->Update();

        vtkNew<vtkTexture> texture;
        texture->SetColorModeToDirectScalars();
        texture->MipmapOn();
        texture->InterpolateOn();
        texture->SetInputConnection(reader->GetOutputPort());

        // 8-bit textures are usually gamma-corrected
        if (reader->GetOutput() && reader->GetOutput()->GetScalarType() == VTK_UNSIGNED_CHAR)
        {
          texture->UseSRGBColorSpaceOn();
        }

        // HDRI OpenGL
        this->UseImageBasedLightingOn();
        this->SetEnvironmentTexture(texture);

        // Skybox OpenGL
        this->Skybox->SetProjection(vtkSkybox::Sphere);
        this->Skybox->SetTexture(texture);

        // First version of VTK including the version check (and the feature used)
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200527)
        this->Skybox->GammaCorrectOn();
#endif
      }
      else
      {
        F3DLog::Print(F3DLog::Severity::Warning, "Cannot open HDRI file ", this->HDRIFile);
      }
    }
  }

  // parse up vector
  std::string up = options.get<std::string>("up");
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

  // skybox orientation
  double front[3];
  vtkMath::Cross(this->RightVector, this->UpVector, front);
  this->Skybox->SetFloorPlane(this->UpVector[0], this->UpVector[1], this->UpVector[2], 0.0);
  this->Skybox->SetFloorRight(front[0], front[1], front[2]);

  if (this->GetUseImageBasedLighting())
  {
    this->AddActor(this->Skybox);
    this->AutomaticLightCreationOff();
  }
  else
  {
    if (options.get<bool>("no-background") && !options.get<std::string>("output").empty())
    {
      // we need to set the background to black to avoid blending issues with translucent
      // objects when saving to file with no background
      this->SetBackground(0, 0, 0);
    }
    else
    {
      this->SetBackground(options.get<std::vector<double> >("background-color").data());
    }
    this->AutomaticLightCreationOn();
  }

  double textColor[3];
  if (this->IsBackgroundDark())
  {
    textColor[0] = textColor[1] = textColor[2] = 1.0;
  }
  else
  {
    textColor[0] = textColor[1] = textColor[2] = 0.0;
  }

  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, fileInfo.c_str());
  this->FilenameActor->GetTextProperty()->SetColor(textColor);

  this->MetaDataActor->GetTextProperty()->SetFontSize(15);
  this->MetaDataActor->GetTextProperty()->SetOpacity(0.5);
  this->MetaDataActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->MetaDataActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->TimerActor->GetTextProperty()->SetColor(textColor);
  this->TimerActor->GetTextProperty()->SetFontSize(15);
  this->TimerActor->SetPosition(10, 10);

  this->CheatSheetActor->GetTextProperty()->SetFontSize(15);
  this->CheatSheetActor->GetTextProperty()->SetOpacity(0.5);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundColor(0, 0, 0);
  this->CheatSheetActor->GetTextProperty()->SetBackgroundOpacity(0.5);

  this->FilenameActor->GetTextProperty()->SetFontFamilyToCourier();
  this->MetaDataActor->GetTextProperty()->SetFontFamilyToCourier();
  this->TimerActor->GetTextProperty()->SetFontFamilyToCourier();
  this->CheatSheetActor->GetTextProperty()->SetFontFamilyToCourier();

  std::string fontFile = options.get<std::string>("font-file");
  if (!fontFile.empty())
  {
    fontFile = vtksys::SystemTools::CollapseFullPath(fontFile);
    if (vtksys::SystemTools::FileExists(fontFile, true))
    {
      this->FilenameActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->FilenameActor->GetTextProperty()->SetFontFile(fontFile.c_str());
      this->MetaDataActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->MetaDataActor->GetTextProperty()->SetFontFile(fontFile.c_str());
      this->TimerActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->TimerActor->GetTextProperty()->SetFontFile(fontFile.c_str());
      this->CheatSheetActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
      this->CheatSheetActor->GetTextProperty()->SetFontFile(fontFile.c_str());
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Cannot find \"", fontFile, "\" font file.");
    }
  }

  this->TimerActor->SetInput("0 fps");

  this->SetupRenderPasses();
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
  newPass->SetForceOpaqueBackground(!this->HDRIFile.empty());

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

  bool hasHDRI = this->GetEnvironmentTexture() != nullptr;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210123)
  vtkOSPRayRendererNode::SetBackgroundMode(
    hasHDRI ? vtkOSPRayRendererNode::Environment : vtkOSPRayRendererNode::Backplate, this);
#else
  vtkOSPRayRendererNode::SetBackgroundMode(hasHDRI ? 2 : 1, this);
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
void vtkF3DRenderer::ShowOptions()
{
  this->ShowGrid(this->GridVisible);
  this->ShowAxis(this->AxisVisible);
  this->ShowTimer(this->TimerVisible);
  this->ShowEdge(this->EdgesVisible);
  this->ShowFilename(this->FilenameVisible);
  this->ShowCheatSheet(this->CheatSheetVisible);
  this->ShowMetaData(this->MetaDataVisible);

  this->UpdateInternalActors();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowAxis(bool show)
{
  if (show)
  {
    vtkNew<vtkAxesActor> axes;
    this->AxisWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    this->AxisWidget->SetOrientationMarker(axes);
    this->AxisWidget->SetInteractor(this->RenderWindow->GetInteractor());
    this->AxisWidget->SetViewport(0.85, 0.0, 1.0, 0.15);
    this->AxisWidget->On();
    this->AxisWidget->InteractiveOff();
    this->AxisWidget->SetKeyPressActivation(false);
  }
  else
  {
    this->AxisWidget = nullptr;
  }

  this->AxisVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsAxisVisible()
{
  return this->AxisVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowGrid(bool show)
{
  this->GridVisible = show;

  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);

  vtkBoundingBox bbox(bounds);

  if (bbox.IsValid())
  {
    this->SetClippingRangeExpansion(0.99);

    double diag = bbox.GetDiagonalLength();
    double unitSquare = pow(10.0, round(log10(diag * 0.1)));

    double gridPos[3];
    for (int i = 0; i < 3; i++)
    {
      double size = bounds[2 * i + 1] - bounds[2 * i];
      gridPos[i] = 0.5 * (bounds[2 * i] + bounds[2 * i + 1] - this->UpVector[i] * size);
    }

    if (this->Verbose && show)
    {
      F3DLog::Print(F3DLog::Severity::Info, "Using grid unit square size = ", unitSquare, "\n",
        "Grid origin set to [", gridPos[0], ", ", gridPos[1], ", ", gridPos[2], "]\n");
    }

    vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
    gridMapper->SetFadeDistance(diag);
    gridMapper->SetUnitSquare(unitSquare);
    gridMapper->SetUpIndex(this->UpIndex);

    this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    this->GridActor->ForceTranslucentOn();
    this->GridActor->SetPosition(gridPos);
    this->GridActor->SetMapper(gridMapper);
    this->GridActor->UseBoundsOff();

    this->RemoveActor(this->GridActor);
    this->AddActor(this->GridActor);
  }
  else
  {
    this->SetClippingRangeExpansion(0);
    show = false;
  }
  this->GridActor->SetVisibility(show);
  this->ResetCameraClippingRange();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsGridVisible()
{
  return this->GridVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseDepthPeelingPass(bool use)
{
  this->UseDepthPeelingPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingBlurBackground()
{
  return this->UseBlurBackground;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseBlurBackground(bool use)
{
  this->UseBlurBackground = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingDepthPeelingPass()
{
  return this->UseDepthPeelingPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseSSAOPass(bool use)
{
  this->UseSSAOPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingSSAOPass()
{
  return this->UseSSAOPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseFXAAPass(bool use)
{
  this->UseFXAAPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingFXAAPass()
{
  return this->UseFXAAPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseToneMappingPass(bool use)
{
  this->UseToneMappingPass = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingToneMappingPass()
{
  return this->UseToneMappingPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracing(bool use)
{
  this->UseRaytracing = use;
  this->UpdateInternalActors();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingRaytracing()
{
  return this->UseRaytracing;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracingDenoiser(bool use)
{
  this->UseRaytracingDenoiser = use;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingRaytracingDenoiser()
{
  return this->UseRaytracingDenoiser;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowTimer(bool show)
{
  if (this->TimerActor)
  {
    this->RemoveActor(this->TimerActor);
    this->AddActor(this->TimerActor);
    this->TimerActor->SetVisibility(show);
  }
  this->TimerVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsTimerVisible()
{
  return this->TimerVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowFilename(bool show)
{
  if (this->FilenameActor)
  {
    this->RemoveActor(this->FilenameActor);
    this->AddActor(this->FilenameActor);
    this->FilenameActor->SetVisibility(show);
  }
  this->FilenameVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsFilenameVisible()
{
  return this->FilenameVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowMetaData(bool show)
{
  if (this->MetaDataActor)
  {
    this->RemoveActor(this->MetaDataActor);
    this->AddActor(this->MetaDataActor);

    // generate field data description
    std::string MetaDataDesc = this->GenerateMetaDataDescription();
    this->MetaDataActor->SetText(vtkCornerAnnotation::RightEdge, MetaDataDesc.c_str());

    this->MetaDataActor->SetVisibility(show);
  }
  this->MetaDataVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsMetaDataVisible()
{
  return this->MetaDataVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowCheatSheet(bool show)
{
  if (this->CheatSheetActor)
  {
    this->RemoveActor(this->CheatSheetActor);

    if (show)
    {
      this->AddActor(this->CheatSheetActor);
      this->CheatSheetActor->SetVisibility(show);
    }
  }
  this->CheatSheetVisible = show;
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsCheatSheetVisible()
{
  return this->CheatSheetVisible;
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
    cheatSheetText << "   ?  : Dump camera state to the terminal\n";
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
  cheatSheetText << " E: Edge visibility " << (this->EdgesVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " X: Axis " << (this->AxisVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " G: Grid " << (this->GridVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " N: File name " << (this->FilenameVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " M: Metadata " << (this->MetaDataVisible ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " Z: FPS Timer " << (this->TimerVisible ? "[ON]" : "[OFF]") << "\n";
#if F3D_MODULE_RAYTRACING
  cheatSheetText << " R: Raytracing " << (this->UseRaytracing ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " D: Denoiser " << (this->UseRaytracingDenoiser ? "[ON]" : "[OFF]") << "\n";
#endif
  cheatSheetText << " F: Full screen "
                 << (this->GetRenderWindow()->GetFullScreen() ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " U: Blur background " << (this->UseBlurBackground ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " K: Trackball interaction " << (this->UseTrackball ? "[ON]" : "[OFF]") << "\n";
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowEdge(bool show)
{
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
  this->EdgesVisible = show;
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsEdgeVisible()
{
  return this->EdgesVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseTrackball(bool use)
{
  this->UseTrackball = use;
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingTrackball()
{
  return this->UseTrackball;
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

  glBeginQuery(GL_TIME_ELAPSED, this->Timer);

  this->TimerActor->RenderOpaqueGeometry(this); // update texture

  this->Superclass::Render();

  auto cpuElapsed = std::chrono::high_resolution_clock::now() - cpuStart;

  // Get CPU frame per seconds
  int fps = static_cast<int>(std::round(
    1.0 / (std::chrono::duration_cast<std::chrono::microseconds>(cpuElapsed).count() * 1e-6)));

  glEndQuery(GL_TIME_ELAPSED);
  GLint elapsed;
  glGetQueryObjectiv(this->Timer, GL_QUERY_RESULT, &elapsed);

  // Get min between CPU frame per seconds and GPU frame per seconds
  fps = std::min(fps, static_cast<int>(std::round(1.0 / (elapsed * 1e-9))));

  std::string str = std::to_string(fps);
  str += " fps";
  this->TimerActor->SetInput(str.c_str());
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::InitializeCamera()
{
  this->ResetCameraClippingRange();
  this->InitialCamera->DeepCopy(this->GetActiveCamera());
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ResetCamera()
{
  vtkCamera* cam = this->GetActiveCamera();
  cam->DeepCopy(this->InitialCamera);
  cam->Modified();
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsBackgroundDark()
{
  double luminance =
    0.299 * this->Background[0] + 0.587 * this->Background[1] + 0.114 * this->Background[2];
  return luminance < 0.5;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::DumpSceneState()
{
  vtkCamera* cam = this->GetActiveCamera();
  double position[3];
  double focal[3];
  double up[3];
  cam->GetPosition(position);
  cam->GetFocalPoint(focal);
  cam->GetViewUp(up);
  F3DLog::Print(
    F3DLog::Severity::Info, "Camera position: ", position[0], ",", position[1], ",", position[2]);
  F3DLog::Print(
    F3DLog::Severity::Info, "Camera focal point: ", focal[0], ",", focal[1], ",", focal[2]);
  F3DLog::Print(F3DLog::Severity::Info, "Camera view up: ", up[0], ",", up[1], ",", up[2], "\n");
}
