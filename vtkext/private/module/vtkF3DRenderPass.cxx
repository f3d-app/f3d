#include "vtkF3DRenderPass.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DHexagonalBokehBlurPass.h"

#include <vtkBoundingBox.h>
#include <vtkCameraPass.h>
#include <vtkDualDepthPeelingPass.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>
#include <vtkLightsPass.h>
#include <vtkObjectFactory.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOverlayPass.h>
#include <vtkProp.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderState.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSSAOPass.h>
#include <vtkSequencePass.h>
#include <vtkShaderProgram.h>
#include <vtkSkybox.h>
#include <vtkTextureObject.h>
#include <vtkToneMappingPass.h>
#include <vtkTranslucentPass.h>
#include <vtkVersion.h>
#include <vtkVolumetricPass.h>

#if F3D_MODULE_RAYTRACING
#include <vtkOSPRayPass.h>
#endif

#include <sstream>

vtkStandardNewMacro(vtkF3DRenderPass);

vtkInformationKeyMacro(vtkF3DRenderPass, RENDER_UI_ONLY, Integer);

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "UseRaytracing: " << this->UseRaytracing << "\n";
  os << indent << "UseSSAOPass: " << this->UseSSAOPass << "\n";
  os << indent << "UseDepthPeelingPass: " << this->UseDepthPeelingPass << "\n";
  os << indent << "UseBlurBackground: " << this->UseBlurBackground << "\n";
  os << indent << "ForceOpaqueBackground: " << this->ForceOpaqueBackground << "\n";
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->BlendQuadHelper)
  {
    this->BlendQuadHelper->ReleaseGraphicsResources(w);
  }
  if (this->BackgroundPass)
  {
    this->BackgroundPass->ReleaseGraphicsResources(w);
  }
  if (this->MainPass)
  {
    this->MainPass->ReleaseGraphicsResources(w);
  }
  if (this->OverlayPass)
  {
    this->OverlayPass->ReleaseGraphicsResources(w);
  }
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Initialize(const vtkRenderState* s)
{
  // Always updates props from the renderstate
  this->BackgroundProps.clear();
  this->OverlayProps.clear();
  this->MainProps.clear();

  // assign props to the correct pass
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
      this->MainProps.push_back(prop);
    }
    else
    {
      this->OverlayProps.push_back(prop);
    }
  }

  if (this->InitializeTime == this->MTime)
  {
    // already initialized
    return;
  }

  this->ReleaseGraphicsResources(s->GetRenderer()->GetRenderWindow());

  // background pass, setup framebuffer, clear and draw skybox
  vtkNew<vtkOpaquePass> bgP;
  vtkNew<vtkCameraPass> bgCamP;
  bgCamP->SetDelegatePass(bgP);
  this->BackgroundPass = vtkSmartPointer<vtkFramebufferPass>::New();
  this->BackgroundPass->SetColorFormat(vtkTextureObject::Float32);

  if (this->UseBlurBackground)
  {
    vtkNew<vtkF3DHexagonalBokehBlurPass> blur;
    blur->SetCircleOfConfusionRadius(this->CircleOfConfusionRadius);
    blur->SetDelegatePass(bgCamP);
    this->BackgroundPass->SetDelegatePass(blur);
  }
  else
  {
    this->BackgroundPass->SetDelegatePass(bgCamP);
  }

  // overlay pass
  vtkNew<vtkDefaultPass> overlayP;
  vtkNew<vtkCameraPass> overlayCamP;
  overlayCamP->SetDelegatePass(overlayP);

  this->OverlayPass = vtkSmartPointer<vtkFramebufferPass>::New();
  this->OverlayPass->SetDelegatePass(overlayCamP);
  this->OverlayPass->SetColorFormat(vtkTextureObject::Float32);

  // main pass
  if (F3D_MODULE_RAYTRACING && this->UseRaytracing)
  {
#if F3D_MODULE_RAYTRACING
    vtkNew<vtkOSPRayPass> ospP;
    this->MainPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->MainPass->SetDelegatePass(ospP);
    this->MainPass->SetColorFormat(vtkTextureObject::Float32);
#endif
  }
  else
  {
    vtkNew<vtkLightsPass> lightsP;
    vtkNew<vtkOpaquePass> opaqueP;
    vtkNew<vtkTranslucentPass> translucentP;
    vtkNew<vtkVolumetricPass> volumeP;

    vtkNew<vtkRenderPassCollection> collection;
    collection->AddItem(lightsP);

    // opaque passes
    if (this->UseSSAOPass)
    {
      vtkBoundingBox bbox(this->Bounds);
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

    vtkNew<vtkSequencePass> sequence;
    sequence->SetPasses(collection);

    vtkNew<vtkCameraPass> camP;
    camP->SetDelegatePass(sequence);

    this->MainPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->MainPass->SetDelegatePass(camP);
    this->MainPass->SetColorFormat(vtkTextureObject::Float32);

    // Needed because VTK can pick the wrong format with certain drivers
    this->MainPass->SetDepthFormat(vtkTextureObject::Fixed32);
  }

  this->InitializeTime = this->GetMTime();
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Render(const vtkRenderState* s)
{
  this->Initialize(s);

  double bgColor[3];

  vtkRenderer* r = s->GetRenderer();
  vtkInformation* info = r->GetInformation();
  bool uiOnly = info->Has(vtkF3DRenderPass::RENDER_UI_ONLY());

  r->GetBackground(bgColor);

  // force background to full black when generating offscreen layers to avoid blending
  // problems when compositing layers in the Blend() function
  r->SetBackground(0.0, 0.0, 0.0);

  if (!uiOnly)
  {
    vtkRenderState backgroundState(s->GetRenderer());
    backgroundState.SetPropArrayAndCount(
      this->BackgroundProps.data(), static_cast<int>(this->BackgroundProps.size()));
    backgroundState.SetFrameBuffer(s->GetFrameBuffer());

    this->BackgroundPass->Render(&backgroundState);

    vtkRenderState mainState(s->GetRenderer());
    mainState.SetPropArrayAndCount(this->MainProps.data(), static_cast<int>(this->MainProps.size()));
    mainState.SetFrameBuffer(s->GetFrameBuffer());

    this->MainPass->Render(&mainState);
  }

  vtkRenderState overlayState(s->GetRenderer());
  overlayState.SetPropArrayAndCount(
    this->OverlayProps.data(), static_cast<int>(this->OverlayProps.size()));
  overlayState.SetFrameBuffer(s->GetFrameBuffer());

  this->OverlayPass->Render(&overlayState);

  // restore background color before compositing the layers
  r->SetBackground(bgColor);

  this->Blend(s);

  this->NumberOfRenderedProps = this->MainPass->GetNumberOfRenderedProps();
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Blend(const vtkRenderState* s)
{
  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());

  r->Clear();

  // Enable blending with default VTK blending function
  // It is required since external window do not set it up
  renWin->GetState()->vtkglEnable(GL_BLEND);
  renWin->GetState()->vtkglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  if (this->BlendQuadHelper && this->BlendQuadHelper->ShaderChangeValue < this->GetMTime())
  {
    this->BlendQuadHelper = nullptr;
  }

  if (!this->BlendQuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    std::stringstream ssDecl;
    ssDecl << "uniform sampler2D texBackground;\n"
              "uniform sampler2D texOverlay;\n"
              "uniform sampler2D texMain;\n"
              "vec3 toLinear(vec3 color) { return pow(color.rgb, vec3(2.2)); }\n"
              "vec3 toSRGB(vec3 color) { return pow(color.rgb, vec3(1.0 / 2.2)); }\n"
              "//VTK::FSQ::Decl";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", ssDecl.str());

    std::stringstream ssImpl;

    ssImpl << "  vec4 mainSample = texture(texMain, texCoord);\n";

    // vtkOSPRayPass output convention changed
    // It was sRGB before and it's now is linear color space
    // Thus, on newer versions, we can skip the conversion to linear space
    // See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/8577
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20230824) || !F3D_MODULE_RAYTRACING
    if (!this->UseRaytracing)
#endif
    {
      // the input color is alpha premultiplied, we need to divide it, then
      // convert to linear space, and finally premultiply back by the alpha value
      ssImpl << "  if (mainSample.a > 0.0)\n";
      ssImpl << "    mainSample.rgb = toLinear(mainSample.rgb / mainSample.a);\n";
      ssImpl << "  mainSample.rgb *= mainSample.a;\n";
    }

    // blend main with background
    if (this->ForceOpaqueBackground)
    {
      ssImpl << "  vec3 bgSample = texture(texBackground, texCoord).rgb;\n";
      ssImpl << "  bgSample = toLinear(bgSample);\n";

      // alpha blending main to opaque background
      ssImpl << "  vec4 result = vec4((1.0 - mainSample.a) * bgSample + mainSample.rgb, 1.0);\n";
    }
    else
    {
      // the background is fully transparent, skip the blending
      ssImpl << "  vec4 result = mainSample;\n";
    }

    // the input color is alpha premultiplied, we need to divide it, then
    // convert to linear space, and finally premultiply back by the alpha value
    ssImpl << "  vec4 ovlSample = texture(texOverlay, texCoord);\n";
    ssImpl << "  if (ovlSample.a > 0.0)\n";
    ssImpl << "    ovlSample.rgb = toLinear(ovlSample.rgb / ovlSample.a);\n";
    ssImpl << "  ovlSample.rgb *= ovlSample.a;\n";

    // premultiplied alpha blending (overlay)
    ssImpl << "  result.rgb = (1.0 - ovlSample.a) * result.rgb + ovlSample.rgb;\n";
    ssImpl << "  result.a = (1.0 - ovlSample.a) * result.a + ovlSample.a;\n";

    // divide by alpha and convert back to sRGB
    // we shouldn't premultiply by alpha again here because the OpenGL blending
    // function is expecting the source fragment not premultiplied
    ssImpl << "  if (result.a > 0.0)\n";
    ssImpl << "    result.rgb = result.rgb / result.a;\n";
    ssImpl << "  result.rgb = toSRGB(result.rgb);\n";

    ssImpl << "  gl_FragData[0] = result;\n";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl", ssImpl.str());

    this->BlendQuadHelper = std::make_shared<vtkOpenGLQuadHelper>(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), FSSource.c_str(), "");

    this->BlendQuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->BlendQuadHelper->Program);
  }

  if (!this->BlendQuadHelper->Program || !this->BlendQuadHelper->Program->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the blend shader program.");
    return;
  }

  this->BackgroundPass->GetColorTexture()->SetWrapS(vtkTextureObject::ClampToEdge);
  this->BackgroundPass->GetColorTexture()->SetWrapT(vtkTextureObject::ClampToEdge);

  this->BackgroundPass->GetColorTexture()->Activate();
  this->OverlayPass->GetColorTexture()->Activate();
  this->MainPass->GetColorTexture()->Activate();
  this->BlendQuadHelper->Program->SetUniformi(
    "texBackground", this->BackgroundPass->GetColorTexture()->GetTextureUnit());
  this->BlendQuadHelper->Program->SetUniformi(
    "texOverlay", this->OverlayPass->GetColorTexture()->GetTextureUnit());
  this->BlendQuadHelper->Program->SetUniformi(
    "texMain", this->MainPass->GetColorTexture()->GetTextureUnit());

  this->BlendQuadHelper->Render();

  this->BackgroundPass->GetColorTexture()->Deactivate();
  this->OverlayPass->GetColorTexture()->Deactivate();
  this->MainPass->GetColorTexture()->Deactivate();
}
