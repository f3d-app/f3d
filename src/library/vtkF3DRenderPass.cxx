#include "vtkF3DRenderPass.h"

#include "F3DConfig.h"

#include <vtkBoundingBox.h>
#include <vtkCameraPass.h>
#include <vtkDualDepthPeelingPass.h>
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
#include <vtkVolumetricPass.h>

#if F3D_MODULE_RAYTRACING
#include <vtkOSPRayPass.h>
#endif

#include <sstream>

vtkStandardNewMacro(vtkF3DRenderPass);

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
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
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Initialize(const vtkRenderState* s)
{
  if (this->InitializeTime == this->MTime)
  {
    // already initialized
    return;
  }

  this->ReleaseGraphicsResources(s->GetRenderer()->GetRenderWindow());

  this->BackgroundProps.clear();
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
    else
    {
      this->MainProps.push_back(prop);
    }
  }

  // background pass, setup framebuffer, clear and draw skybox
  vtkNew<vtkOpaquePass> bgP;
  vtkNew<vtkCameraPass> bgCamP;
  bgCamP->SetDelegatePass(bgP);
  this->BackgroundPass = vtkSmartPointer<vtkFramebufferPass>::New();
  this->BackgroundPass->SetDelegatePass(bgCamP);
  this->BackgroundPass->SetColorFormat(vtkTextureObject::Float32);

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
    vtkNew<vtkOverlayPass> overlayP;

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

    collection->AddItem(overlayP);

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

  vtkRenderState backgroundState(s->GetRenderer());
  backgroundState.SetPropArrayAndCount(
    this->BackgroundProps.data(), static_cast<int>(this->BackgroundProps.size()));
  backgroundState.SetFrameBuffer(s->GetFrameBuffer());

  this->BackgroundPass->Render(&backgroundState);

  vtkRenderState mainState(s->GetRenderer());
  mainState.SetPropArrayAndCount(this->MainProps.data(), static_cast<int>(this->MainProps.size()));
  mainState.SetFrameBuffer(s->GetFrameBuffer());

  this->MainPass->Render(&mainState);

  this->Blend(s);

  this->NumberOfRenderedProps = this->MainPass->GetNumberOfRenderedProps();
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Blend(const vtkRenderState* s)
{
  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  r->Clear();
  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglDisable(GL_BLEND);

  if (this->BlendQuadHelper && this->BlendQuadHelper->ShaderChangeValue < this->GetMTime())
  {
    this->BlendQuadHelper = nullptr;
  }

  if (!this->BlendQuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    std::stringstream ssDecl;
    ssDecl << "uniform sampler2D texBackground;\n"
              "uniform sampler2D texMain;\n"
              "#define SAMPLES 200\n"
              "#define GOLDEN_ANGLE 2.399963\n"
              "vec3 BokehBlur(vec2 tcoords, float radius)"
              "{\n"
              "  vec3 acc = vec3(0.0);\n"
              "  float factor = radius / sqrt(float(SAMPLES));\n"
              "  for (int i = 0; i < SAMPLES; i++)\n"
              "  {\n"
              "    float theta = float(i) * GOLDEN_ANGLE;\n"
              "    float r = factor * sqrt(float(i));\n"
              "    vec2 pt = vec2(r * cos(theta), r * sin(theta));\n"
              "    vec3 col = texture(texBackground, tcoords + pt).rgb;\n"
              "    acc += col;\n"
              "  }\n"
              "  return acc / vec3(SAMPLES);\n"
              "}\n"
              "vec4 cubic(float v)\n"
              "{\n"
              "  vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;\n"
              "  vec4 s = n * n * n;\n"
              "  float x = s.x;\n"
              "  float y = s.y - 4.0 * s.x;\n"
              "  float z = s.z - 4.0 * s.y + 6.0 * s.x;\n"
              "  float w = 6.0 - x - y - z;\n"
              "  return vec4(x, y, z, w) * (1.0/6.0);\n"
              "}\n"
              "vec4 textureBicubic(vec2 texCoords)\n"
              "{\n"
              "  vec2 texSize = textureSize(texBackground, 0);\n"
              "  vec2 invTexSize = 1.0 / texSize;\n"
              "  texCoords = texCoords * texSize - 0.5;\n"
              "  vec2 fxy = fract(texCoords);\n"
              "  texCoords -= fxy;\n"
              "  vec4 xcubic = cubic(fxy.x);\n"
              "  vec4 ycubic = cubic(fxy.y);\n"
              "  vec4 c = texCoords.xxyy + vec2(-0.5, 1.5).xyxy;\n"
              "  vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);\n"
              "  vec4 offset = c + vec4(xcubic.yw, ycubic.yw) / s;\n"
              "  offset *= invTexSize.xxyy;\n"
              "  vec4 sample0 = texture(texBackground, offset.xz);\n"
              "  vec4 sample1 = texture(texBackground, offset.yz);\n"
              "  vec4 sample2 = texture(texBackground, offset.xw);\n"
              "  vec4 sample3 = texture(texBackground, offset.yw);\n"
              "  float sx = s.x / (s.x + s.y);\n"
              "  float sy = s.z / (s.z + s.w);\n"
              "  return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);\n"
              "}\n"
              "//VTK::FSQ::Decl";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", ssDecl.str());

    std::stringstream ssImpl;
    ssImpl << "  vec4 mainSample = texture(texMain, texCoord);\n";
    if (this->UseBlurBackground)
    {
      ssImpl << "  vec3 bgCol = BokehBlur(texCoord, 0.05);\n";
    }
    else
    {
      ssImpl << "  vec3 bgCol = textureBicubic(texCoord).rgb;\n";
    }
    ssImpl << "  vec3 result = mix(bgCol, mainSample.rgb, mainSample.a);\n";

    if (this->ForceOpaqueBackground)
    {
      ssImpl << "  gl_FragData[0] = vec4(result.rgb, 1.0);\n";
    }
    else
    {
      ssImpl << "  gl_FragData[0] = vec4(result.rgb, mainSample.a);\n";
    }

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
  this->MainPass->GetColorTexture()->Activate();
  this->BlendQuadHelper->Program->SetUniformi(
    "texBackground", this->BackgroundPass->GetColorTexture()->GetTextureUnit());
  this->BlendQuadHelper->Program->SetUniformi(
    "texMain", this->MainPass->GetColorTexture()->GetTextureUnit());

  this->BlendQuadHelper->Render();

  this->BackgroundPass->GetColorTexture()->Deactivate();
  this->MainPass->GetColorTexture()->Deactivate();
}
