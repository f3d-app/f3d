#include "vtkF3DRenderer.h"

#include "F3DLoader.h"
#include "F3DOptions.h"

#include "vtkF3DOpenGLGridMapper.h"

#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCameraPass.h>
#include <vtkDualDepthPeelingPass.h>
#include <vtkImporter.h>
#include <vtkLightsPass.h>
#include <vtkObjectFactory.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOverlayPass.h>
#include <vtkProperty.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSSAOPass.h>
#include <vtkSequencePass.h>
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
  this->AutomaticLightCreationOn();

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
  this->UseDepthPeeling = this->Options.DepthPeeling;
  this->UseSSAOPass = this->Options.SSAO;
  this->UseFXAAPass = this->Options.FXAA;
  this->UsePointSprites = this->Options.PointSprites;

  this->SetBackground(this->Options.BackgroundColor[0], this->Options.BackgroundColor[1],
    this->Options.BackgroundColor[2]);

  this->FilenameActor->SetText(vtkCornerAnnotation::UpperEdge, fileInfo.c_str());

  this->SetupRenderPasses();

  this->ShowOptions();
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetupRenderPasses()
{
#if F3D_HAS_RAYTRACING
  if (this->UseRaytracing)
  {
    vtkNew<vtkOSPRayPass> osprayP;
    vtkNew<vtkTranslucentPass> translucentP;

    vtkNew<vtkRenderPassCollection> collection;
    collection->AddItem(osprayP);
    collection->AddItem(translucentP);

    vtkNew<vtkSequencePass> sequence;
    sequence->SetPasses(collection);

    vtkNew<vtkCameraPass> cameraP;
    cameraP->SetDelegatePass(sequence);
    this->SetPass(cameraP);

    vtkOSPRayRendererNode::SetRendererType("pathtracer", this);
    vtkOSPRayRendererNode::SetSamplesPerPixel(this->Options.Samples, this);
    vtkOSPRayRendererNode::SetEnableDenoiser(this->UseRaytracingDenoiser, this);
    vtkOSPRayRendererNode::SetDenoiserThreshold(0, this);
    vtkOSPRayRendererNode::SetBackgroundMode(1, this);

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
  if (this->UseDepthPeeling)
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

  if (this->UseFXAAPass)
  {
    vtkNew<vtkOpenGLFXAAPass> fxaaP;
    fxaaP->SetDelegatePass(cameraP);

    this->SetPass(fxaaP);
  }
  else
  {
    this->SetPass(cameraP);
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
  this->ResetCameraClippingRange();

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
        "Grid origin set to [", gridX, ", ", gridY, ", ", gridZ, "]");
    }

    vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
    gridMapper->SetFadeDistance(diag);
    gridMapper->SetUnitSquare(unitSquare);

    this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    this->GridActor->ForceTranslucentOn();
    this->GridActor->SetPosition(gridX, gridY, gridZ);
    this->GridActor->SetMapper(gridMapper);

    this->RemoveActor(this->GridActor);
    this->AddActor(this->GridActor);
  }
  else
  {
    show = false;
  }
  this->GridActor->SetVisibility(show);
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::IsGridVisible()
{
  return this->GridVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRenderer::SetUseDepthPeelingPass(bool use)
{
  this->UseDepthPeeling = use;
  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
bool vtkF3DRenderer::UsingDepthPeeling()
{
  return this->UseDepthPeeling;
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
