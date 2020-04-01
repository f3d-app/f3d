#include "vtkF3DRenderer.h"

#include "F3DLoader.h"
#include "F3DOptions.h"

#include "vtkF3DOpenGLGridMapper.h"

#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkCameraPass.h>
#include <vtkDualDepthPeelingPass.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImporter.h>
#include <vtkLightsPass.h>
#include <vtkObjectFactory.h>
#include <vtkToneMappingPass.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLTexture.h>
#include <vtkOverlayPass.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSSAOPass.h>
#include <vtkSequencePass.h>
#include <vtkSkybox.h>
#include <vtkTranslucentPass.h>
#include <vtkVolumetricPass.h>
#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <vtk_glew.h>

#if F3D_HAS_RAYTRACING
#include <vtkOSPRayPass.h>
#include <vtkOSPRayRendererNode.h>
#endif

#include "F3DLog.h"

#include <cmath>

vtkStandardNewMacro(vtkF3DRenderer);

//----------------------------------------------------------------------------
vtkF3DRenderer::~vtkF3DRenderer()
{
  if (this->Timer != 0)
  {
    glDeleteQueries(1, &this->Timer);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Initialize(const F3DOptions& options, const std::string& fileInfo)
{
  if (!this->RenderWindow)
  {
    F3DLog::Print(F3DLog::Severity::Error, "No render window linked");
    return;
  }

  if (this->Timer == 0)
  {
    glGenQueries(1, &this->Timer);
  }

  this->RemoveAllViewProps();
  this->RemoveAllLights();

  this->Options = options;

  this->GridVisible = this->Options.Grid;
  this->AxisVisible = this->Options.Axis;
  this->EdgesVisible = this->Options.Edges;
  this->TimerVisible = this->Options.FPS;
  this->FilenameVisible = this->Options.Filename;
  this->ScalarBarVisible = this->Options.Bar;
  this->ScalarsVisible = !this->Options.Scalars.empty();
  this->UseRaytracing = this->Options.Raytracing;
  this->UseRaytracingDenoiser = this->Options.Denoise;
  this->UseDepthPeelingPass = this->Options.DepthPeeling;
  this->UseSSAOPass = this->Options.SSAO;
  this->UseFXAAPass = this->Options.FXAA;
  this->UseToneMappingPass = this->Options.ToneMapping;
  this->UsePointSprites = this->Options.PointSprites;

  if (!this->Options.HDRIFile.empty() && !this->GetUseImageBasedLighting())
  {
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(this->Options.HDRIFile);

    auto reader = vtkSmartPointer<vtkImageReader2>::Take(
      vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));
    if (reader)
    {
      reader->SetFileName(fullPath.c_str());
      reader->Update();
      vtkNew<vtkTexture> texture;
      texture->SetColorModeToDirectScalars();
      texture->SetInputConnection(reader->GetOutputPort());

      // HDRI OpenGL
      this->UseImageBasedLightingOn();
      this->SetEnvironmentTexture(texture, true);

      // Skybox OpenGL
      this->Skybox->SetFloorRight(0.0, 0.0, 1.0);
      this->Skybox->SetProjection(vtkSkybox::Sphere);
      this->Skybox->SetTexture(texture);
    }
    else
    {
      vtkWarningMacro("Cannot open HDRI file " << fullPath);
    }
  }

  if (this->GetUseImageBasedLighting())
  {
    this->AddActor(this->Skybox);
    this->AutomaticLightCreationOff();
  }
  else
  {
    this->SetBackground(this->Options.BackgroundColor[0], this->Options.BackgroundColor[1],
      this->Options.BackgroundColor[2]);
    this->AutomaticLightCreationOn();
  }

  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, fileInfo.c_str());

  this->ShowOptions();

  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetupRenderPasses()
{
  vtkRenderPass* pass = this->GetPass();
  if (pass)
  {
    pass->ReleaseGraphicsResources(this->RenderWindow);
  }

#if F3D_HAS_RAYTRACING
  if (this->UseRaytracing)
  {
    vtkNew<vtkOSPRayPass> osprayP;
    this->SetPass(osprayP);

    vtkOSPRayRendererNode::SetRendererType("pathtracer", this);
    vtkOSPRayRendererNode::SetSamplesPerPixel(this->Options.Samples, this);
    vtkOSPRayRendererNode::SetEnableDenoiser(this->UseRaytracingDenoiser, this);
    vtkOSPRayRendererNode::SetDenoiserThreshold(0, this);

    bool hasHDRI = this->GetEnvironmentTexture() != nullptr;
    vtkOSPRayRendererNode::SetBackgroundMode(hasHDRI ? 2 : 1, this);

    return;
  }
#endif

  vtkNew<vtkLightsPass> lightsP;
  vtkNew<vtkOpaquePass> opaqueP;
  vtkNew<vtkTranslucentPass> translucentP;
  vtkNew<vtkVolumetricPass> volumeP;
  vtkNew<vtkOverlayPass> overlayP;

  vtkNew<vtkRenderPassCollection> collection;
  collection->AddItem(lightsP);

  // opaque passes
  if (this->UseSSAOPass)
  {
    double bounds[6];
    this->ComputeVisiblePropBounds(bounds);

    vtkBoundingBox bbox(bounds);

    if (bbox.IsValid())
    {
      vtkNew<vtkCameraPass> ssaoCamP;
      ssaoCamP->SetDelegatePass(opaqueP);

      vtkNew<vtkSSAOPass> ssaoP;
      ssaoP->SetRadius(0.1 * bbox.GetDiagonalLength());
      ssaoP->SetBias(0.001 * bbox.GetDiagonalLength());
      ssaoP->SetKernelSize(200);
      ssaoP->SetDelegatePass(ssaoCamP);

      collection->AddItem(ssaoP);
    }
    else
    {
      collection->AddItem(opaqueP);
    }
  }
  else
  {
    collection->AddItem(opaqueP);
  }

  // translucent and volumic passes
  if (this->UseDepthPeelingPass)
  {
    vtkNew<vtkDualDepthPeelingPass> ddpP;
    ddpP->SetTranslucentPass(translucentP);
    ddpP->SetVolumetricPass(volumeP);
    collection->AddItem(ddpP);
  }
  else
  {
    collection->AddItem(translucentP);
    collection->AddItem(volumeP);
  }

  collection->AddItem(overlayP);

  vtkNew<vtkSequencePass> sequence;
  sequence->SetPasses(collection);

  vtkNew<vtkCameraPass> cameraP;
  cameraP->SetDelegatePass(sequence);

  vtkSmartPointer<vtkRenderPass> activePass = cameraP;
  if (this->UseToneMappingPass)
  {
    vtkNew<vtkToneMappingPass> toneP;
    toneP->SetToneMappingType(vtkToneMappingPass::GenericFilmic);
    toneP->SetGenericFilmicDefaultPresets();
    toneP->SetDelegatePass(cameraP);
    activePass = toneP;
  }

  if (this->UseFXAAPass)
  {
    vtkNew<vtkOpenGLFXAAPass> fxaaP;
    fxaaP->SetDelegatePass(activePass);

    this->SetPass(fxaaP);
  }
  else
  {
    this->SetPass(activePass);
  }
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
  }
  else
  {
    this->AxisWidget = nullptr;
  }

  this->AxisVisible = show;
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

  this->SetClippingRangeExpansion(0.99);

  double bounds[6];
  this->ComputeVisiblePropBounds(bounds);

  vtkBoundingBox bbox(bounds);

  if (bbox.IsValid())
  {
    double diag = bbox.GetDiagonalLength();
    double unitSquare = pow(10.0, round(log10(diag * 0.1)));

    double gridX = 0.5 * (bounds[0] + bounds[1]);
    double gridY = bounds[2];
    double gridZ = 0.5 * (bounds[4] + bounds[5]);

    if (this->Options.Verbose && show)
    {
      F3DLog::Print(F3DLog::Severity::Info, "Using grid unit square size = ", unitSquare, "\n",
        "Grid origin set to [", gridX, ", ", gridY, ", ", gridZ, "]\n");
    }

    vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
    gridMapper->SetFadeDistance(diag);
    gridMapper->SetUnitSquare(unitSquare);

    this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    this->GridActor->ForceTranslucentOn();
    this->GridActor->SetPosition(gridX, gridY, gridZ);
    this->GridActor->SetMapper(gridMapper);
    this->GridActor->UseBoundsOff();

    this->RemoveActor(this->GridActor);
    this->AddActor(this->GridActor);
  }
  else
  {
    show = false;
  }
  this->GridActor->SetVisibility(show);
  this->ResetCameraClippingRange();
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
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingToneMappingPass()
{
  return this->UseToneMappingPass;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUsePointSprites(bool use)
{
  this->UsePointSprites = use;
  if (this->GeometryActor && this->PointGaussianMapper && this->PolyDataMapper)
  {
    if (use)
    {
      this->GeometryActor->SetMapper(this->PointGaussianMapper);
    }
    else
    {
      this->GeometryActor->SetMapper(this->PolyDataMapper);
    }
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingPointSprites()
{
  return this->UsePointSprites;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseRaytracing(bool use)
{
  this->UseRaytracing = use;

  if (this->GeometryActor && this->PointGaussianMapper && this->PolyDataMapper)
  {
    if (use)
    {
      this->GeometryActor->SetMapper(this->PolyDataMapper);
    }
    else
    {
      this->SetUsePointSprites(this->UsePointSprites);
    }
  }

  this->SetupRenderPasses();
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
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingRaytracingDenoiser()
{
  return this->UseRaytracingDenoiser;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowScalars(bool show)
{
  this->ScalarsVisible = show;
  if (this->GeometryActor && this->PointGaussianMapper && this->PolyDataMapper)
  {
    this->PolyDataMapper->SetScalarVisibility(show);
    this->PointGaussianMapper->SetScalarVisibility(show);
    this->ShowScalarBar(this->ScalarBarVisible);
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::AreScalarsVisible()
{
  return this->ScalarsVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowScalarBar(bool show)
{
  this->ScalarBarVisible = show;
  if (this->ScalarBarActor)
  {
    this->ScalarBarActor->SetVisibility(show && this->ScalarsVisible);
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsScalarBarVisible()
{
  return this->ScalarBarVisible;
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
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsFilenameVisible()
{
  return this->FilenameVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowEdge(bool show)
{
  vtkActor* anActor;
  vtkActorCollection* ac = this->GetActors();
  vtkCollectionSimpleIterator ait;
  for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait));)
  {
    anActor->GetProperty()->SetEdgeVisibility(show);
  }
  this->EdgesVisible = show;
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsEdgeVisible()
{
  return this->EdgesVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ShowOptions()
{
  this->ShowGrid(this->GridVisible);
  this->ShowAxis(this->AxisVisible);
  this->ShowScalarBar(this->ScalarBarVisible);
  this->ShowScalars(this->ScalarsVisible);
  this->ShowTimer(this->TimerVisible);
  this->ShowEdge(this->EdgesVisible);
  this->ShowFilename(this->FilenameVisible);

  // Set the initial camera once all options 
  // have been shown as they may have an effect on it
  vtkCamera* cam = this->GetActiveCamera();
  this->InitialCamera->DeepCopy(cam);
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::Render()
{
  if (this->Timer > 0)
  {
    glBeginQuery(GL_TIME_ELAPSED, this->Timer);
  }

  this->Superclass::Render();

  if (this->Timer > 0)
  {
    glEndQuery(GL_TIME_ELAPSED);
    GLint elapsed;
    glGetQueryObjectiv(this->Timer, GL_QUERY_RESULT, &elapsed);
    int fps = static_cast<int>(std::round(1.0 / (elapsed * 1e-9)));
    std::string str = std::to_string(fps);
    str += " fps";
    this->TimerActor->SetInput(str.c_str());
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::InitializeCamera()
{
  this->Superclass::ResetCamera();
  vtkCamera* cam = this->GetActiveCamera();
  if (this->Options.CameraPosition.size() == 3)
  {
    cam->SetPosition(this->Options.CameraPosition.data());
  }
  if (this->Options.CameraFocalPoint.size() == 3)
  {
    cam->SetFocalPoint(this->Options.CameraFocalPoint.data());
  }
  if (this->Options.CameraViewUp.size() == 3)
  {
    cam->SetViewUp(this->Options.CameraViewUp.data());
  }
  if (this->Options.CameraViewAngle != 0)
  {
    cam->SetViewAngle(this->Options.CameraViewAngle);
  }
  cam->OrthogonalizeViewUp();
  if (this->Options.Verbose)
  {
    double* position = cam->GetPosition();
    F3DLog::Print(F3DLog::Severity::Info, "Camera position is : ", position[0], ", ", position[1], ", ", position[2], ".");
    double* focalPoint = cam->GetFocalPoint();
    F3DLog::Print(F3DLog::Severity::Info, "Camera focal point is : ", focalPoint[0], ", ", focalPoint[1], ", ", focalPoint[2], ".");
    double* viewUp = cam->GetViewUp();
    F3DLog::Print(F3DLog::Severity::Info, "Camera view up is : ", viewUp[0], ", ", viewUp[1], ", ", viewUp[2], ".");
    F3DLog::Print(F3DLog::Severity::Info, "Camera view angle is : ", cam->GetViewAngle(), ".\n");
  }
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::ResetCamera()
{
  vtkCamera* cam = this->GetActiveCamera();
  cam->DeepCopy(this->InitialCamera);
  cam->Modified();
}
