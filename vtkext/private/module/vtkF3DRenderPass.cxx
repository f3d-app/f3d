#include "vtkF3DRenderPass.h"

#include "F3DLog.h"
#include "vtkF3DBakeSSAOPass.h"
#include "vtkF3DDepthPrePass.h"
#include "vtkF3DFixColorSpacePass.h"
#include "vtkF3DImporter.h"
#include "vtkF3DMainRenderPass.h"
#include "vtkF3DModulateSSAOPass.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DPolyDataMapper.h"
#include "vtkF3DPostprocessRenderPass.h"
#include "vtkF3DReflectionRenderPass.h"
#include "vtkF3DRenderer.h"
#include "vtkF3DSkinningMorphingPass.h"
#include "vtkF3DStochasticTransparentPass.h"
#include "vtkF3DTAAPass.h"
#include "vtkF3DUserEvents.h"

#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCameraPass.h>
#include <vtkDefaultPass.h>
#include <vtkDualDepthPeelingPass.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>
#include <vtkInteractorObserver.h>
#include <vtkLightsPass.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLFXAAPass.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkPointData.h>
#include <vtkProp.h>
#include <vtkProp3D.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderState.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSequencePass.h>
#include <vtkShaderProgram.h>
#include <vtkShaderProperty.h>
#include <vtkShadowMapBakerPass.h>
#include <vtkShadowMapPass.h>

#include <vtkSkybox.h>
#include <vtkTextureObject.h>
#include <vtkTranslucentPass.h>
#include <vtkUniforms.h>
#include <vtkVersion.h>
#include <vtkVolumetricPass.h>

#if F3D_MODULE_ANARI
#include <vtkAnariPass.h>
#endif

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
  os << indent << "UseAnari: " << this->UseAnari << "\n";
  os << indent << "UseSSAOPass: " << this->UseSSAOPass << "\n";
  os << indent << "UseBlurBackground: " << this->UseBlurBackground << "\n";
  os << indent << "UseToneMappingPass: " << this->UseToneMappingPass << "\n";
  os << indent << "ForceOpaqueBackground: " << this->ForceOpaqueBackground << "\n";
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->PostprocessPass)
  {
    this->PostprocessPass->ReleaseGraphicsResources(w);
  }
  if (this->DepthPrePass)
  {
    this->DepthPrePass->ReleaseGraphicsResources(w);
  }
  if (this->BakeSSAOPass)
  {
    this->BakeSSAOPass->ReleaseGraphicsResources(w);
  }
  if (this->BakeReflectionPass)
  {
    this->BakeReflectionPass->ReleaseGraphicsResources(w);
  }
  if (this->MainPass)
  {
    this->MainPass->ReleaseGraphicsResources(w);
  }
  if (this->FXAAPass)
  {
    this->FXAAPass->ReleaseGraphicsResources(w);
  }
}

// ----------------------------------------------------------------------------
vtkSmartPointer<vtkSequencePass> vtkF3DRenderPass::CreatePropsSequence(
  vtkF3DRenderer* renderer, bool useDualDepthPeeling, bool modulateSSAO)
{
  const vtkNew<vtkLightsPass> lightsP;
  const vtkNew<vtkOpaquePass> opaqueP;
  const vtkNew<vtkTranslucentPass> translucentP;
  const vtkNew<vtkVolumetricPass> volumeP;

  // the props rendering passes, wrapped below in the passes modifying their shaders
  vtkNew<vtkRenderPassCollection> collection;

  // opaque passes
  if (modulateSSAO)
  {
    vtkNew<vtkF3DModulateSSAOPass> modulateSSAOP;
    modulateSSAOP->SetSSAOTexture(this->BakeSSAOPass->GetSSAOTexture());
    modulateSSAOP->SetDelegatePass(opaqueP);
    collection->AddItem(modulateSSAOP);
  }
  else
  {
    collection->AddItem(opaqueP);
  }

  // translucent and volumic
  if (useDualDepthPeeling)
  {
#ifndef F3D_USE_GLES
    vtkNew<vtkDualDepthPeelingPass> ddpP;
    ddpP->SetTranslucentPass(translucentP);
    ddpP->SetVolumetricPass(volumeP);
    collection->AddItem(ddpP);
#endif
  }
  else if (renderer && renderer->GetBlendingMode() == vtkF3DRenderer::BlendingMode::STOCHASTIC)
  {
    vtkNew<vtkF3DStochasticTransparentPass> stochasticP;
    stochasticP->SetTranslucentPass(translucentP);
    stochasticP->SetVolumetricPass(volumeP);
    collection->AddItem(stochasticP);
  }
  else
  {
    collection->AddItem(translucentP);
    collection->AddItem(volumeP);
  }

  vtkNew<vtkSequencePass> propsSequence;
  propsSequence->SetPasses(collection);

  const vtkNew<vtkF3DSkinningMorphingPass> skinningMorphingP;

  if (renderer && !renderer->GetSRGBBlending())
  {
    const vtkNew<vtkF3DFixColorSpacePass> fixColorSpaceP;
    fixColorSpaceP->SetDelegatePass(propsSequence);
    skinningMorphingP->SetDelegatePass(fixColorSpaceP);
  }
  else
  {
    skinningMorphingP->SetDelegatePass(propsSequence);
  }

  vtkNew<vtkRenderPassCollection> mainCollection;
  mainCollection->AddItem(lightsP);
  mainCollection->AddItem(skinningMorphingP);

  auto sequence = vtkSmartPointer<vtkSequencePass>::New();
  sequence->SetPasses(mainCollection);
  return sequence;
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Initialize(const vtkRenderState* s)
{
  if (this->InitializeTime == this->MTime)
  {
    // already initialized
    return;
  }

  vtkOpenGLRenderer* glRenderer = vtkOpenGLRenderer::SafeDownCast(s->GetRenderer());
  vtkF3DRenderer* renderer = vtkF3DRenderer::SafeDownCast(glRenderer);
  vtkOpenGLRenderWindow* glRenderWindow =
    vtkOpenGLRenderWindow::SafeDownCast(glRenderer->GetRenderWindow());

  this->ReleaseGraphicsResources(glRenderWindow);
  this->BakeSSAOPass = nullptr;

  // depth pre-pass, write normals and depth of opaque actors, it owns the depth texture
  this->DepthPrePass = vtkSmartPointer<vtkF3DDepthPrePass>::New();

  const vtkNew<vtkOpaquePass> depthOpaqueP;

  const vtkNew<vtkF3DSkinningMorphingPass> depthSkinningMorphingP;
  depthSkinningMorphingP->SetDelegatePass(depthOpaqueP);

  this->DepthPrePass->SetDelegatePass(depthSkinningMorphingP);

  // overlay pass
  this->OverlayPass = vtkSmartPointer<vtkDefaultPass>::New();

  // main pass
  this->MainPass = vtkSmartPointer<vtkF3DMainRenderPass>::New();
  this->MainPass->SetArmatureVisible(this->ArmatureVisible);
  this->MainPass->SetUseRaytracing(this->UseRaytracing);
  this->MainPass->SetUseBlurBackground(this->UseBlurBackground);
  this->MainPass->SetCircleOfConfusionRadius(this->CircleOfConfusionRadius);
  this->MainPass->SetDepthTexture(this->DepthPrePass->GetDepthTexture());

#if F3D_MODULE_RAYTRACING
  if (this->UseRaytracing)
  {
    vtkNew<vtkOSPRayPass> ospP;
    this->MainPass->SetDelegatePass(ospP);
  }
  else
#endif
#ifdef F3D_MODULE_ANARI
    if (this->UseAnari)
  {
    vtkNew<vtkAnariPass> anariP;
    anariP->GetAnariDevice()->SetupAnariDeviceFromLibrary(
      this->AnariLibrary.c_str(), "default", false);
    this->MainPass->SetDelegatePass(anariP);
  }
  else
#endif
  {
    // SSAO is baked from the depth pre-pass buffers
    const vtkBoundingBox bbox(this->Bounds);
    if (this->UseSSAOPass && bbox.IsValid())
    {
      this->BakeSSAOPass = vtkSmartPointer<vtkF3DBakeSSAOPass>::New();
      this->BakeSSAOPass->SetRadius(0.1 * bbox.GetDiagonalLength());
      this->BakeSSAOPass->SetBias(0.001 * bbox.GetDiagonalLength());
    }

    bool useDualDepthPeeling =
      renderer && renderer->GetBlendingMode() == vtkF3DRenderer::BlendingMode::DUAL_DEPTH_PEELING;
#ifdef F3D_USE_GLES
    if (useDualDepthPeeling)
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Dual depth peeling is not supported on GLES. Ignored."));
      useDualDepthPeeling = false;
    }
#endif

    const vtkSmartPointer<vtkSequencePass> sequence =
      this->CreatePropsSequence(renderer, useDualDepthPeeling, this->BakeSSAOPass != nullptr);

    vtkSmartPointer<vtkRenderPass> mainSequencePass = sequence;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20261016)
    if (this->RenderShadows)
    {
      vtkNew<vtkCameraPass> camP;
      camP->SetDelegatePass(sequence);

      // shadows
      const vtkNew<vtkShadowMapBakerPass> shadowBakeP;
      shadowBakeP->SetExponentialConstant(this->ShadowExponentialConstant);
      shadowBakeP->SetResolution(this->ShadowResolution);
      shadowBakeP->SetDelegatePass(camP);

      const vtkNew<vtkShadowMapPass> shadowP;
      shadowP->SetDelegatePass(sequence);
      shadowP->SetShadowMapBakerPass(shadowBakeP);

      vtkNew<vtkRenderPassCollection> collectionShadows;
      collectionShadows->AddItem(shadowBakeP);
      collectionShadows->AddItem(shadowP);

      vtkNew<vtkSequencePass> sequenceShadows;
      sequenceShadows->SetPasses(collectionShadows);

      mainSequencePass = sequenceShadows;
    }
#endif

    // TAA
    if (renderer && renderer->GetAntiAliasingMode() == vtkF3DRenderer::AntiAliasingMode::TAA)
    {
      vtkNew<vtkF3DTAAPass> taaP;
      taaP->SetDelegatePass(mainSequencePass);

      glRenderer->GetRenderWindow()->AddObserver(
        vtkCommand::WindowResizeEvent, taaP.Get(), &vtkF3DTAAPass::ResetIterations);
      glRenderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle()->AddObserver(
        vtkCommand::InteractionEvent, taaP.Get(), &vtkF3DTAAPass::ResetIterations);
      glRenderer->AddObserver(
        vtkF3DUserEvents::InvalidateHistoryEvent, taaP.Get(), &vtkF3DTAAPass::ResetIterations);

      this->MainPass->SetDelegatePass(taaP);
    }
    else
    {
      this->MainPass->SetDelegatePass(mainSequencePass);
    }

    // reflection baking pass, same as main pass but with reflected camera
    this->BakeReflectionPass = vtkSmartPointer<vtkF3DReflectionRenderPass>::New();

    // the ambient occlusion is baked for the main camera, it does not apply to the reflection
    this->BakeReflectionPass->SetDelegatePass(
      this->BakeSSAOPass && this->RenderReflection && renderer
        ? this->CreatePropsSequence(renderer, useDualDepthPeeling, false)
        : sequence);

    if (this->RenderReflection && renderer)
    {
      this->MainPass->SetReflectionPass(this->BakeReflectionPass);
    }

    vtkProp* const* props = s->GetPropArray();
    for (int i = 0; i < s->GetPropArrayCount(); i++)
    {
      vtkActor* actor = vtkActor::SafeDownCast(props[i]);
      if (actor)
      {
        vtkF3DOpenGLGridMapper* gridMapper =
          vtkF3DOpenGLGridMapper::SafeDownCast(actor->GetMapper());
        if (gridMapper)
        {
          gridMapper->SetReflectionColorTexture(this->BakeReflectionPass->GetColorTexture());
          gridMapper->SetReflectionDepthTexture(this->BakeReflectionPass->GetDepthTexture());
        }
      }
    }
  }

  // post-process pass
  this->PostprocessPass = vtkSmartPointer<vtkF3DPostprocessRenderPass>::New();
  this->FXAAPass = nullptr;

  // FXAA wraps the post-process pass only, before the overlay
  if (renderer && renderer->GetAntiAliasingMode() == vtkF3DRenderer::AntiAliasingMode::FXAA)
  {
    this->FXAAPass = vtkSmartPointer<vtkOpenGLFXAAPass>::New();
  }

  const double scale = renderer ? renderer->GetScreenSpaceScaling() : 1.0;
  this->DepthPrePass->SetResolutionScale(scale);
  this->MainPass->SetResolutionScale(scale);
  if (this->BakeReflectionPass)
  {
    this->BakeReflectionPass->SetResolutionScale(scale);
  }
  this->PostprocessPass->SetResolutionScale(scale);

  this->PostprocessPass->SetUserShader(this->UserShader);
  this->PostprocessPass->SetColorMap(this->ColorMap);
  this->PostprocessPass->SetUseToneMapping(this->UseToneMappingPass);
  this->PostprocessPass->SetColorInputIsLinear(!renderer || !renderer->GetSRGBBlending());

  if (this->FXAAPass)
  {
    this->FXAAPass->SetDelegatePass(this->PostprocessPass);
  }

  this->InitializeTime = this->GetMTime();
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::ReplaceMatCapShader(
  std::string& fragmentShader, vtkActor* actor, vtkPolyData* polyData)
{
  if (polyData && polyData->GetPointData()->GetNormals() != nullptr) // check if we have normals
  {
    auto textures = actor->GetProperty()->GetAllTextures();
    auto fn = [](const std::pair<std::string, vtkTexture*>& tex) { return tex.first == "matcap"; };
    const bool hasMatcap = std::ranges::find_if(textures, fn) != textures.end();

    if (hasMatcap)
    {
      // disable PBR light, just sample matcap and set final color to gamma-corrected ambient color

      const std::string customColor =
        "  //VTK::Color::Impl\n"
#ifdef F3D_USE_GLES
        "  vec2 uv = vec2(vertexNormalVCVS.xy) * 0.5 + vec2(0.5,0.5);\n"
#else
        "  vec2 uv = vec2(normalVCVSOutput.xy) * 0.5 + vec2(0.5,0.5);\n"
#endif
        "  diffuseColor = vec3(0.0);\n"
        "  ambientColor = texture(matcap, uv).rgb;\n";

      vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Color::Impl", customColor);

      const std::string customLight =
        "  gl_FragData[0] = vec4(pow(ambientColor, vec3(1.0/2.2)), 1.0);\n";

      vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Light::Impl", customLight);

      // disable default behavior of VTK with textures to avoid blending with itself
      vtkShaderProgram::Substitute(fragmentShader, "//VTK::TCoord::Impl", "");
    }
  }
}

// ----------------------------------------------------------------------------
bool vtkF3DRenderPass::PreReplaceShaderValues(std::string& vertexShader, std::string&,
  std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop)
{
  vtkPolyDataMapper* polyMapper = vtkPolyDataMapper::SafeDownCast(mapper);
  vtkActor* actor = vtkActor::SafeDownCast(prop);

  if (!polyMapper || !actor)
  {
    return true;
  }

  vtkPolyData* input = polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;

  this->ReplaceMatCapShader(fragmentShader, actor, input);

  if (!actor->GetProperty()->GetLighting() && actor->GetProperty()->GetInterpolation() != VTK_PBR)
  {
    // apply final gamma-correction
    const std::string customGamma =
      "//VTK::TCoord::Impl\n"
      "gl_FragData[0] = vec4(pow(gl_FragData[0].rgb, vec3(1.0/2.2)), gl_FragData[0].a);\n";

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::TCoord::Impl", customGamma);
  }

  return true;
}

// ----------------------------------------------------------------------------
bool vtkF3DRenderPass::PostReplaceShaderValues([[maybe_unused]] std::string& vertexShader,
  std::string&, std::string& fragmentShader, [[maybe_unused]] vtkAbstractMapper* mapper,
  [[maybe_unused]] vtkProp* prop)
{
  // Scale the HDRI image-based lighting (diffuse + specular) by IBLIntensity so it
  // follows render.light.intensity just like explicit lights do.
  // The IBLIntensity uniform is declared automatically via VTK's //VTK::CustomUniforms::Dec
  // mechanism: vtkF3DRenderer::UpdateLights calls SetUniformf("IBLIntensity", ...) on every
  // actor each frame so the declaration is always present when the shader is compiled.
  // This substitution lives here (in the render pass, not the mapper) so it applies to
  // both the standard OpenGL mapper and the GLES mapper used for web and Android.
  // see https://github.com/f3d-app/f3d/issues/3312
  if (fragmentShader.find("iblDiffuse") != std::string::npos)
  {
    vtkShaderProgram::Substitute(fragmentShader, "vec3 color = iblDiffuse + iblSpecular;",
      "vec3 color = (iblDiffuse + iblSpecular) * IBLIntensity;");
  }
  return true;
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLRenderWindow* renWin =
    vtkOpenGLRenderWindow::SafeDownCast(s->GetRenderer()->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();
  ostate->PushFramebufferBindings();

  this->PreRender(s);

  this->Initialize(s);

  vtkRenderer* r = s->GetRenderer();
  vtkInformation* info = r->GetInformation();
  const bool uiOnly = info->Has(vtkF3DRenderPass::RENDER_UI_ONLY());

  if (!uiOnly)
  {
    // the props are filtered here as the depth pre-pass needs them before the main pass is rendered
    this->MainPass->FilterProps(s);

    // the depth pre-pass must be the first pass to run, it is not relevant with external renderers
    if (!this->UseRaytracing && !this->UseAnari)
    {
      this->DepthPrePass->Render(s);

      if (this->BakeSSAOPass)
      {
        this->BakeSSAOPass->SetDepthTexture(this->DepthPrePass->GetDepthTexture());
        this->BakeSSAOPass->SetNormalTexture(this->DepthPrePass->GetNormalTexture());
        this->BakeSSAOPass->Render(s);
      }
    }

    const vtkF3DRenderer* renderer = vtkF3DRenderer::SafeDownCast(r);
    if (this->BakeReflectionPass && renderer)
    {
      this->BakeReflectionPass->SetReflectionMatrix(renderer->GetGridMatrix());
    }

    this->MainPass->Render(s);
  }

  // FXAA requires the default framebuffer to be bound
  // TODO: apply FXAA in the post-process directly with our own shader
  ostate->PopFramebufferBindings();

  this->PostprocessPass->SetColorTexture(this->MainPass->GetColorTexture());
  this->PostprocessPass->SetDepthTexture(this->DepthPrePass->GetDepthTexture());
  this->PostprocessPass->SetNormalTexture(this->DepthPrePass->GetNormalTexture());
  if (this->FXAAPass)
  {
    this->FXAAPass->Render(s);
  }
  else
  {
    this->PostprocessPass->Render(s);
  }

  std::vector<vtkProp*> overlayProps;
  vtkProp* const* props = s->GetPropArray();
  for (int i = 0; i < s->GetPropArrayCount(); i++)
  {
    if (!vtkProp3D::SafeDownCast(props[i]))
    {
      overlayProps.push_back(props[i]);
    }
  }
  vtkRenderState overlayState(r);
  overlayState.SetPropArrayAndCount(overlayProps.data(), static_cast<int>(overlayProps.size()));
  overlayState.SetFrameBuffer(s->GetFrameBuffer());
  this->OverlayPass->Render(&overlayState);

  this->NumberOfRenderedProps = this->MainPass->GetNumberOfRenderedProps();

  this->PostRender(s);
}
