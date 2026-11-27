#include "vtkF3DMainRenderPass.h"

#include "vtkF3DHexagonalBokehBlurPass.h"
#include "vtkF3DImporter.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DReflectionRenderPass.h"
#include "vtkF3DRenderPassUtils.h"

#include <vtkActor.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3D.h>
#include <vtkProperty.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkSkybox.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DMainRenderPass);

//------------------------------------------------------------------------------
vtkF3DMainRenderPass::vtkF3DMainRenderPass()
{
  this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
}

//------------------------------------------------------------------------------
vtkF3DMainRenderPass::~vtkF3DMainRenderPass() = default;

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::SetDelegatePass(vtkRenderPass* delegate)
{
  if (this->DelegatePass != delegate)
  {
    this->DelegatePass = delegate;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::SetReflectionPass(vtkF3DReflectionRenderPass* reflectionPass)
{
  if (this->ReflectionPass != reflectionPass)
  {
    this->ReflectionPass = reflectionPass;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::FilterProps(const vtkRenderState* s)
{
  this->BackgroundProps.clear();
  this->MainOnTopProps.clear();
  this->MainProps.clear();

  vtkProp** props = s->GetPropArray();
  for (int i = 0; i < s->GetPropArrayCount(); i++)
  {
    vtkProp* prop = props[i];
    if (vtkSkybox::SafeDownCast(prop))
    {
      this->BackgroundProps.push_back(prop);
    }
    else if (vtkProp3D::SafeDownCast(prop))
    {
      // armature
      vtkInformation* info = prop->GetPropertyKeys();
      if (info && info->Has(vtkF3DImporter::ACTOR_IS_ARMATURE()))
      {
        if (this->ArmatureVisible)
        {
          this->MainOnTopProps.push_back(prop);
        }
      }
      else
      {
        vtkActor* actor = vtkActor::SafeDownCast(prop);

        if (actor)
        {
          vtkPolyDataMapper* polyMapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
          if (polyMapper)
          {
#if F3D_MODULE_RAYTRACING
            // Enable linear colorspace conversion during raytracing on PBR actors.
            if (auto* mapper = vtkF3DPolyDataMapper::SafeDownCast(polyMapper))
            {
              mapper->SetUseLinearColorSpace(
                this->UseRaytracing && actor->GetProperty()->GetInterpolation() == VTK_PBR);
            }
#endif
            polyMapper->SetVBOShiftScaleMethod(
              vtkPolyDataMapper::ShiftScaleMethodType::DISABLE_SHIFT_SCALE);

            vtkPolyData* input =
              polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;

            if (input && input->GetNumberOfPoints() == 0)
            {
              // skip rendering if no point
              continue;
            }
          }
        }

        this->MainProps.push_back(prop);
      }
    }
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::SetDepthTexture(vtkTextureObject* depthTexture)
{
  if (this->DepthTexture != depthTexture)
  {
    this->DepthTexture = depthTexture;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  this->FrameBufferObject = nullptr;
  this->ColorTexture->ReleaseGraphicsResources(w);

  if (this->OnTopDepthTexture)
  {
    this->OnTopDepthTexture->ReleaseGraphicsResources(w);
    this->OnTopDepthTexture = nullptr;
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::RenderOnTopProps(
  vtkRenderer* r, vtkOpenGLRenderWindow* renWin, int width, int height)
{
  if (this->MainOnTopProps.empty())
  {
    // free the memory if the on top props are not visible anymore
    if (this->OnTopDepthTexture)
    {
      this->OnTopDepthTexture->ReleaseGraphicsResources(renWin);
      this->OnTopDepthTexture = nullptr;
    }
    return;
  }

  // lazy allocation, only needed with on top props
  if (!this->OnTopDepthTexture)
  {
    this->OnTopDepthTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->OnTopDepthTexture->SetContext(renWin);
    this->OnTopDepthTexture->AllocateDepth(width, height, vtkTextureObject::Float32);
  }
  this->OnTopDepthTexture->Resize(width, height);

  // swap the depth buffer so the main one is not modified
  this->FrameBufferObject->AddDepthAttachment(this->OnTopDepthTexture);

  vtkF3DRenderPassUtils::SetRenderOptions(vtkOpenGLRenderer::SafeDownCast(r),
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestLEqual,
      .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .ClearDepth = 1.f,
      .WriteDepth = true,
      .Width = width,
      .Height = height });

  for (vtkProp* prop : this->MainOnTopProps)
  {
    prop->RenderOpaqueGeometry(r);
    this->NumberOfRenderedProps++;
  }

  // restore the main depth buffer
  this->FrameBufferObject->AddDepthAttachment(this->DepthTexture);
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height)
{
  assert(this->DepthTexture);

  this->ColorTexture->SetContext(renWin);
  if (!this->ColorTexture->GetHandle())
  {
    this->ColorTexture->SetInternalFormat(GL_RGBA16F);
    this->ColorTexture->SetDataType(GL_FLOAT);
    this->ColorTexture->Create2D(width, height, 4, VTK_UNSIGNED_CHAR, false);
  }
  this->ColorTexture->Resize(width, height);

  if (!this->FrameBufferObject)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
  }
  this->FrameBufferObject->SetContext(renWin);

  const vtkNew<vtkOpaquePass> bgP;
  if (this->UseBlurBackground)
  {
    vtkNew<vtkF3DHexagonalBokehBlurPass> blurP;
    blurP->SetCircleOfConfusionRadius(this->CircleOfConfusionRadius);
    blurP->SetDelegatePass(bgP);
    this->BackgroundPass = blurP;
  }
  else
  {
    this->BackgroundPass = bgP;
  }
}

//------------------------------------------------------------------------------
void vtkF3DMainRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  if (!this->DelegatePass)
  {
    vtkWarningMacro(<< " no delegate.");
    return;
  }

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(r->GetRenderWindow());

  int width = 0;
  int height = 0;
  vtkF3DRenderPassUtils::ComputeRenderSize(s, this->ResolutionScale, width, height);

  this->InitializeResources(renWin, width, height);

  this->FilterProps(s);

  if (this->ReflectionPass)
  {
    // the reflection result is used when rendering the main props so it must be rendered before
    vtkRenderState reflState(r);
    reflState.SetPropArrayAndCount(
      this->MainProps.data(), static_cast<int>(this->MainProps.size()));
    reflState.SetFrameBuffer(s->GetFrameBuffer());

    this->ReflectionPass->Render(&reflState);
  }

  renWin->GetState()->PushFramebufferBindings();

  this->FrameBufferObject->Bind();
  this->FrameBufferObject->AddColorAttachment(0, this->ColorTexture);
  this->FrameBufferObject->AddDepthAttachment(this->DepthTexture);
  this->FrameBufferObject->StartNonOrtho(width, height);

  vtkOpenGLRenderer* glRenderer = vtkOpenGLRenderer::SafeDownCast(r);

  // do not clear the depth buffer, it comes from the prepass
  vtkF3DRenderPassUtils::SetRenderOptions(glRenderer,
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestLEqual,
      .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .ClearColor = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f },
      .Width = width,
      .Height = height });

  vtkRenderState s2(s->GetRenderer());
  s2.SetFrameBuffer(this->FrameBufferObject);

  // TODO: disable depth test if TAA only? clear color if no skybox only?
  vtkF3DRenderPassUtils::SetRenderOptions(glRenderer,
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestNone,
      .ClearColor = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f },
      .Width = width,
      .Height = height });

  // render skybox
  // will use the depth buffer from the prepass to limit overdraw
  // must be run first for proper compositing with the main translucent props
  s2.SetPropArrayAndCount(
    this->BackgroundProps.data(), static_cast<int>(this->BackgroundProps.size()));
  this->BackgroundPass->Render(&s2);
  this->NumberOfRenderedProps += this->BackgroundPass->GetNumberOfRenderedProps();

  vtkF3DRenderPassUtils::SetRenderOptions(glRenderer,
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestLEqual,
      .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .WriteDepth = true, // needed? maybe with stochastic?
      .Width = width,
      .Height = height });

  // render main props
  s2.SetPropArrayAndCount(this->MainProps.data(), static_cast<int>(this->MainProps.size()));
  this->DelegatePass->Render(&s2);
  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();

  // render "on top" props, depth tested between themselves only
  this->RenderOnTopProps(r, renWin, width, height);

  renWin->GetState()->PopFramebufferBindings();

  vtkOpenGLCheckErrorMacro("failed after Render");
}
