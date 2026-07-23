#include "vtkF3DRenderPass.h"

#include "F3DLog.h"
#include "vtkF3DHexagonalBokehBlurPass.h"
#include "vtkF3DImporter.h"
#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderer.h"
#include "vtkF3DStochasticTransparentPass.h"
#include "vtkF3DTAAPass.h"

#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkCameraPass.h>
#include <vtkDualDepthPeelingPass.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>
#include <vtkInteractorObserver.h>
#include <vtkLightsPass.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkOverlayPass.h>
#include <vtkPointData.h>
#include <vtkProp.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderState.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSSAOPass.h>
#include <vtkSequencePass.h>
#include <vtkShaderProgram.h>
#include <vtkShaderProperty.h>
#include <vtkSkybox.h>
#include <vtkTextureObject.h>
#include <vtkToneMappingPass.h>
#include <vtkTranslucentPass.h>
#include <vtkUniforms.h>
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
  if (this->BakeReflectionPass)
  {
    this->BakeReflectionPass->ReleaseGraphicsResources(w);
  }
  if (this->MainPass)
  {
    this->MainPass->ReleaseGraphicsResources(w);
  }
  if (this->MainOnTopPass)
  {
    this->MainOnTopPass->ReleaseGraphicsResources(w);
  }
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Initialize(const vtkRenderState* s)
{
  // Always updates props from the renderstate
  this->BackgroundProps.clear();
  this->MainOnTopProps.clear();
  this->MainProps.clear();
  this->ReflectionProps.clear();

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
            polyMapper->SetVBOShiftScaleMethod(
              vtkPolyDataMapper::ShiftScaleMethodType::DISABLE_SHIFT_SCALE);

            vtkPolyData* input =
              polyMapper->GetNumberOfInputPorts() > 0 ? polyMapper->GetInput() : nullptr;

            // handle skinning/morphing attributes for the actor
            if (input)
            {
              if (input->GetNumberOfPoints() == 0)
              {
                // skip rendering if no point
                continue;
              }

              // skinning
              if (input->GetPointData()->GetArray("WEIGHTS_0") != nullptr &&
                input->GetPointData()->GetArray("JOINTS_0") != nullptr)
              {
                // map glTF arrays to GPU VBOs
                polyMapper->MapDataArrayToVertexAttribute(
                  "weights", "WEIGHTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);
                polyMapper->MapDataArrayToVertexAttribute(
                  "joints", "JOINTS_0", vtkDataObject::FIELD_ASSOCIATION_POINTS);
              }

              // morph targets
              // OpenGL limits the input attributes to 16 vectors
              // We ignore morphing on tangent on purpose to maximize the number of targets we
              // can support
              for (int j = 0; j < 4; j++)
              {
                std::string namePosition = "target" + std::to_string(j) + "_position";

                if (input->GetPointData()->GetArray(namePosition.c_str()) == nullptr)
                {
                  break;
                }

                polyMapper->MapDataArrayToVertexAttribute(namePosition.c_str(),
                  namePosition.c_str(), vtkDataObject::FIELD_ASSOCIATION_POINTS);

                std::string nameNormal = "target" + std::to_string(j) + "_normal";
                if (input->GetPointData()->GetArray(nameNormal.c_str()) != nullptr)
                {
                  polyMapper->MapDataArrayToVertexAttribute(nameNormal.c_str(), nameNormal.c_str(),
                    vtkDataObject::FIELD_ASSOCIATION_POINTS);
                }
              }
            }
          }
        }

        this->MainProps.push_back(prop);

        if (!actor || vtkF3DOpenGLGridMapper::SafeDownCast(actor->GetMapper()) == nullptr)
        {
          // Do not add the grid into the reflective actors
          this->ReflectionProps.push_back(prop);
        }
      }
    }
  }

  if (this->InitializeTime == this->MTime)
  {
    // already initialized
    return;
  }

  vtkOpenGLRenderer* glRenderer = vtkOpenGLRenderer::SafeDownCast(s->GetRenderer());
  this->LightComplexity = glRenderer->GetLightingComplexity();

  this->ReleaseGraphicsResources(glRenderer->GetRenderWindow());

  // background pass, setup framebuffer, clear and draw skybox
  vtkNew<vtkOpaquePass> bgP;
  vtkNew<vtkCameraPass> bgCamP;
  bgCamP->SetDelegatePass(bgP);
  this->BackgroundPass = vtkSmartPointer<vtkFramebufferPass>::New();
  this->BackgroundPass->SetColorFormat(vtkTextureObject::Float16);

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

  // main pass
#if F3D_MODULE_RAYTRACING
  if (this->UseRaytracing)
  {
    vtkNew<vtkOSPRayPass> ospP;
    this->MainPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->MainPass->SetDelegatePass(ospP);
    this->MainPass->SetColorFormat(vtkTextureObject::Float32);
  }
  else
#endif
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

    // translucent and volumic
    const vtkF3DRenderer* renderer = vtkF3DRenderer::SafeDownCast(glRenderer);
    if (renderer && renderer->GetBlendingMode() == vtkF3DRenderer::BlendingMode::DUAL_DEPTH_PEELING)
    {
#ifndef F3D_USE_GLES
      vtkNew<vtkDualDepthPeelingPass> ddpP;
      ddpP->SetTranslucentPass(translucentP);
      ddpP->SetVolumetricPass(volumeP);
      collection->AddItem(ddpP);
#else
      F3DLog::Print(F3DLog::Severity::Warning,
        std::string("Dual depth peeling is not supported on GLES. Ignored."));
      collection->AddItem(translucentP);
      collection->AddItem(volumeP);
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

    vtkNew<vtkSequencePass> sequence;
    sequence->SetPasses(collection);

    vtkNew<vtkCameraPass> camP;
    camP->SetDelegatePass(sequence);

    this->MainPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->MainPass->SetColorFormat(vtkTextureObject::Float32);

    // Needed because VTK can pick the wrong format with certain drivers
#ifdef F3D_USE_GLES
    // Fixed32 not supported on GLES
    this->MainPass->SetDepthFormat(vtkTextureObject::Fixed24);
#else
    this->MainPass->SetDepthFormat(vtkTextureObject::Fixed32);
#endif

    // TAA
    if (renderer && renderer->GetAntiAliasingMode() == vtkF3DRenderer::AntiAliasingMode::TAA)
    {
      vtkNew<vtkF3DTAAPass> taaP;
      taaP->SetDelegatePass(camP);

      glRenderer->GetRenderWindow()->AddObserver(
        vtkCommand::WindowResizeEvent, taaP.Get(), &vtkF3DTAAPass::ResetIterations);
      glRenderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle()->AddObserver(
        vtkCommand::InteractionEvent, taaP.Get(), &vtkF3DTAAPass::ResetIterations);

      this->MainPass->SetDelegatePass(taaP);
    }
    else
    {
      this->MainPass->SetDelegatePass(camP);
    }

    // reflection baking pass, same as main pass but with reflected camera
    this->BakeReflectionPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->BakeReflectionPass->SetColorFormat(vtkTextureObject::Float16);
    this->BakeReflectionPass->SetDelegatePass(camP);

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

  {
    vtkNew<vtkLightsPass> lightsP;
    vtkNew<vtkOpaquePass> opaqueP;

    vtkNew<vtkRenderPassCollection> collection;
    collection->AddItem(lightsP);
    collection->AddItem(opaqueP);

    vtkNew<vtkSequencePass> sequence;
    sequence->SetPasses(collection);

    vtkNew<vtkCameraPass> camP;
    camP->SetDelegatePass(sequence);

    this->MainOnTopPass = vtkSmartPointer<vtkFramebufferPass>::New();
    this->MainOnTopPass->SetDelegatePass(camP);

    // Needed because VTK can pick the wrong format with certain drivers
#ifdef F3D_USE_GLES
    // Fixed32 not supported on GLES
    this->MainOnTopPass->SetDepthFormat(vtkTextureObject::Fixed24);
#else
    this->MainOnTopPass->SetDepthFormat(vtkTextureObject::Fixed32);
#endif
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
    bool hasMatcap = std::ranges::find_if(textures, fn) != textures.end();

    if (hasMatcap)
    {
      // disable PBR light, just sample matcap and set final color to gamma-corrected ambient color

      std::string customColor = "  //VTK::Color::Impl\n"
#ifdef F3D_USE_GLES
                                "  vec2 uv = vec2(vertexNormalVCVS.xy) * 0.5 + vec2(0.5,0.5);\n"
#else
                                "  vec2 uv = vec2(normalVCVSOutput.xy) * 0.5 + vec2(0.5,0.5);\n"
#endif
                                "  diffuseColor = vec3(0.0);\n"
                                "  ambientColor = texture(matcap, uv).rgb;\n";

      vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Color::Impl", customColor);

      std::string customLight = "  gl_FragData[0] = vec4(pow(ambientColor, vec3(1.0/2.2)), 1.0);\n";

      vtkShaderProgram::Substitute(fragmentShader, "  //VTK::Light::Impl", customLight);

      // disable default behavior of VTK with textures to avoid blending with itself
      vtkShaderProgram::Substitute(fragmentShader, "//VTK::TCoord::Impl", "");
    }
  }
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::ReplaceSkinningMorphing(
  std::string& vertexShader, vtkActor* actor, vtkPolyData* polyData)
{
  if (polyData)
  {
    vtkUniforms* uniforms = actor->GetShaderProperty()->GetVertexCustomUniforms();
    bool hasMorphing =
      uniforms->GetUniformTupleType("morphWeights") != vtkUniforms::TupleTypeInvalid;
    bool hasSkinning =
      uniforms->GetUniformTupleType("jointMatrices") != vtkUniforms::TupleTypeInvalid;

    if (hasMorphing || hasSkinning)
    {
      bool hasTangents =
        polyData->GetPointData()->GetTangents() && actor->GetProperty()->GetLighting();
      bool hasNormals =
        polyData->GetPointData()->GetNormals() && actor->GetProperty()->GetLighting();
      hasTangents = hasTangents && (actor->GetProperty()->GetTexture("normalTex") != nullptr);

      std::string customDecl = "//VTK::CustomUniforms::Dec\n";
      std::string beginImpl = "//VTK::CustomBegin::Impl\n";
      std::string posImpl = "//VTK::PositionVC::Impl\n"
                            "  vec4 posMC = vertexMC;\n";

      // normals and tangents can be modified by skinnning and morphing in model space
      std::string normalImpl = "//VTK::Normal::Impl\n";
      if (hasNormals)
      {
        normalImpl += "  normalVCVSOutput = normalMC;\n";
      }
      if (hasTangents)
      {
#ifndef F3D_USE_GLES
        normalImpl += "  tangentVCVSOutput = tangentMC;\n";
#else
        normalImpl += "  tangentVCVS = tangentMC;\n";
#endif
      }

      // morphing
      if (hasMorphing)
      {
        for (int i = 0; i < 4; i++)
        {
          std::string name = "target" + std::to_string(i) + "_position";

          // modify position using morph weights
          if (polyData->GetPointData()->GetArray(name.c_str()) != nullptr)
          {
#ifndef F3D_USE_GLES
            customDecl += "in vec3 ";
            customDecl += name;
            customDecl += ";\n";

            posImpl += " posMC += morphWeights[";
            posImpl += std::to_string(i);
            posImpl += "] * vec4(";
            posImpl += name;
            posImpl += ", 0);\n";
#else
            customDecl += "uniform sampler2D ";
            customDecl += name;
            customDecl += ";\n";

            posImpl += " posMC += morphWeights[";
            posImpl += std::to_string(i);
            posImpl += "] * vec4(texelFetchBuffer(";
            posImpl += name;
            posImpl += ", pointId).xyz, 0);\n";
#endif
          }

          name = "target" + std::to_string(i) + "_normal";

          // modify normal using morph weights
          if (polyData->GetPointData()->GetArray(name.c_str()) != nullptr)
          {
#ifndef F3D_USE_GLES
            customDecl += "in vec3 ";
            customDecl += name;
            customDecl += ";\n";

            if (hasNormals)
            {
              normalImpl += " normalVCVSOutput += morphWeights[";
              normalImpl += std::to_string(i);
              normalImpl += "] * ";
              normalImpl += name;
              normalImpl += ";\n";
            }
#else
            customDecl += "uniform sampler2D ";
            customDecl += name;
            customDecl += ";\n";

            if (hasNormals)
            {
              normalImpl += " normalVCVSOutput += morphWeights[";
              normalImpl += std::to_string(i);
              normalImpl += "] * texelFetchBuffer(";
              normalImpl += name;
              normalImpl += ", pointId).xyz;\n";
            }
#endif
          }
        }
      }

      // skin
      if (hasSkinning)
      {
        // automatically added with GLES
#ifndef F3D_USE_GLES
        customDecl += "in vec4 joints;\n"
                      "in vec4 weights;\n";
#else
        customDecl += "uniform mediump isampler2D joints;\n"
                      "uniform mediump sampler2D weights;\n";
#endif

#ifndef F3D_USE_GLES
        beginImpl += "  vec4 currentWeight = weights;\n"
                     "  ivec4 currentJoints = ivec4(joints);\n";
#else
        beginImpl += "  vec4 currentWeight = texelFetchBuffer(weights, pointId);\n"
                     "  ivec4 currentJoints = texelFetchBuffer(joints, pointId);\n";
#endif

        // compute skinning matrix with current uniform weights
        beginImpl += "  mat4 skinMat = currentWeight.x * jointMatrices[currentJoints.x]\n"
                     "               + currentWeight.y * jointMatrices[currentJoints.y]\n"
                     "               + currentWeight.z * jointMatrices[currentJoints.z]\n"
                     "               + currentWeight.w * jointMatrices[currentJoints.w];\n";

        posImpl += "  posMC = skinMat * posMC;\n";

        // apply the matrix to normals and tangents
        if (hasNormals)
        {
          normalImpl += "  normalVCVSOutput = mat3(skinMat) * normalVCVSOutput;\n";
        }
        if (hasTangents)
        {
#ifndef F3D_USE_GLES
          normalImpl += "  tangentVCVSOutput = mat3(skinMat) * tangentVCVSOutput;\n";
#else
          normalImpl += "  tangentVCVS = mat3(skinMat) * tangentVCVS;\n";
#endif
        }
      }

      posImpl += "  gl_Position = MCDCMatrix * posMC;\n";

      if (this->LightComplexity > 0)
      {
        posImpl += "  vertexVCVSOutput = MCVCMatrix * posMC;\n";
      }

      if (hasNormals)
      {
        normalImpl += "  normalVCVSOutput = normalMatrix * normalVCVSOutput;\n";
#ifdef F3D_USE_GLES
        // for some reason GLES shader has two normal outputs
        normalImpl += "  normalVCVSInput = normalVCVSOutput;\n";
#endif
      }
      if (hasTangents)
      {
#ifndef F3D_USE_GLES
        normalImpl += "  tangentVCVSOutput = normalMatrix * tangentVCVSOutput;\n";
#else
        normalImpl += "  tangentVCVS = normalMatrix * tangentVCVS;\n";
#endif
      }

      vtkShaderProgram::Substitute(vertexShader, "//VTK::CustomUniforms::Dec", customDecl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::PositionVC::Impl", posImpl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::Normal::Impl", normalImpl);
      vtkShaderProgram::Substitute(vertexShader, "//VTK::CustomBegin::Impl", beginImpl);
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
    std::string customGamma =
      "//VTK::TCoord::Impl\n"
      "gl_FragData[0] = vec4(pow(gl_FragData[0].rgb, vec3(1.0/2.2)), gl_FragData[0].a);\n";

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::TCoord::Impl", customGamma);
  }

  this->ReplaceSkinningMorphing(vertexShader, actor, input);

  return true;
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Render(const vtkRenderState* s)
{
  this->PreRender(s);

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

    // the reflection result is used in the main pass so it must be rendered before
    vtkF3DRenderer* renderer = vtkF3DRenderer::SafeDownCast(r);

#if F3D_MODULE_RAYTRACING
    if (!this->UseRaytracing)
#endif
    {
      if (this->RenderReflection && renderer != nullptr)
      {
        vtkRenderState reflState(s->GetRenderer());
        reflState.SetPropArrayAndCount(
          this->ReflectionProps.data(), static_cast<int>(this->ReflectionProps.size()));
        reflState.SetFrameBuffer(s->GetFrameBuffer());

        vtkMatrix4x4* actorMatrix = renderer->GetGridMatrix();

        vtkCamera* originalCam = s->GetRenderer()->GetActiveCamera();
        vtkNew<vtkCamera> reflectedCam;

        // reflect camera according to the grid plane
        this->ReflectCamera(originalCam, actorMatrix, reflectedCam);
        r->SetActiveCamera(reflectedCam);

        this->BakeReflectionPass->Render(&reflState);

        // restore camera
        r->SetActiveCamera(originalCam);
      }
    }

    vtkRenderState mainState(s->GetRenderer());
    mainState.SetPropArrayAndCount(
      this->MainProps.data(), static_cast<int>(this->MainProps.size()));
    mainState.SetFrameBuffer(s->GetFrameBuffer());

    this->MainPass->Render(&mainState);

    vtkRenderState mainOnTopState(s->GetRenderer());
    mainOnTopState.SetPropArrayAndCount(
      this->MainOnTopProps.data(), static_cast<int>(this->MainOnTopProps.size()));
    mainOnTopState.SetFrameBuffer(s->GetFrameBuffer());

    this->MainOnTopPass->Render(&mainOnTopState);
  }

  // restore background color before compositing the layers
  r->SetBackground(bgColor);

  this->Blend(s);

  this->NumberOfRenderedProps = this->MainPass->GetNumberOfRenderedProps();

  this->PostRender(s);
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::Blend(const vtkRenderState* s)
{
  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

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
              "uniform sampler2D texMainOnTop;\n"
              "vec3 toLinear(vec3 color) { return pow(color.rgb, vec3(2.2)); }\n"
              "vec3 toSRGB(vec3 color) { return pow(color.rgb, vec3(1.0 / 2.2)); }\n"
              "//VTK::FSQ::Decl";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", ssDecl.str());

    std::stringstream ssImpl;

    ssImpl << "  vec4 mainSample = texture(texMain, texCoord);\n";

    if (!this->UseRaytracing)
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

    // blend "on top"
    ssImpl << "  vec4 onTopSample = texture(texMainOnTop, texCoord);\n";
    ssImpl << "  if (onTopSample.a > 0.0)\n";
    ssImpl << "    onTopSample.rgb = toLinear(onTopSample.rgb / onTopSample.a);\n";
    ssImpl << "  onTopSample.rgb *= onTopSample.a;\n";
    ssImpl << "  result.rgb = (1.0 - onTopSample.a) * result.rgb + onTopSample.rgb;\n";
    ssImpl << "  result.a = (1.0 - onTopSample.a) * result.a + onTopSample.a;\n";

    // divide by alpha and convert back to sRGB, premultiply again
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
  this->MainPass->GetColorTexture()->Activate();
  this->MainOnTopPass->GetColorTexture()->Activate();
  this->BlendQuadHelper->Program->SetUniformi(
    "texBackground", this->BackgroundPass->GetColorTexture()->GetTextureUnit());
  this->BlendQuadHelper->Program->SetUniformi(
    "texMain", this->MainPass->GetColorTexture()->GetTextureUnit());
  this->BlendQuadHelper->Program->SetUniformi(
    "texMainOnTop", this->MainOnTopPass->GetColorTexture()->GetTextureUnit());

  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);

  this->BlendQuadHelper->Render();

  this->BackgroundPass->GetColorTexture()->Deactivate();
  this->MainPass->GetColorTexture()->Deactivate();
  this->MainOnTopPass->GetColorTexture()->Deactivate();
}

// ----------------------------------------------------------------------------
void vtkF3DRenderPass::ReflectCamera(
  vtkCamera* originalCam, vtkMatrix4x4* actorMatrix, vtkCamera* reflectedCam)
{
  // compute the reflection matrix
  // see https://www.opengl.org/archives/resources/code/samples/sig99/advanced99/notes/node159.html
  double n[3] = { actorMatrix->GetElement(0, 1), actorMatrix->GetElement(1, 1),
    actorMatrix->GetElement(2, 1) };
  vtkMath::Normalize(n);

  // Plane center point
  const double p[3] = { actorMatrix->GetElement(0, 3), actorMatrix->GetElement(1, 3),
    actorMatrix->GetElement(2, 3) };

  const double a = n[0], b = n[1], c = n[2];
  const double d = a * p[0] + b * p[1] + c * p[2];

  vtkNew<vtkMatrix4x4> reflectionMatrix;
  reflectionMatrix->SetElement(0, 0, 1 - 2 * a * a);
  reflectionMatrix->SetElement(0, 1, -2 * a * b);
  reflectionMatrix->SetElement(0, 2, -2 * a * c);
  reflectionMatrix->SetElement(0, 3, 2 * a * d);

  reflectionMatrix->SetElement(1, 0, -2 * a * b);
  reflectionMatrix->SetElement(1, 1, 1 - 2 * b * b);
  reflectionMatrix->SetElement(1, 2, -2 * b * c);
  reflectionMatrix->SetElement(1, 3, 2 * b * d);

  reflectionMatrix->SetElement(2, 0, -2 * a * c);
  reflectionMatrix->SetElement(2, 1, -2 * b * c);
  reflectionMatrix->SetElement(2, 2, 1 - 2 * c * c);
  reflectionMatrix->SetElement(2, 3, 2 * c * d);

  reflectionMatrix->SetElement(3, 0, 0);
  reflectionMatrix->SetElement(3, 1, 0);
  reflectionMatrix->SetElement(3, 2, 0);
  reflectionMatrix->SetElement(3, 3, 1);

  // Reflect position
  double pos[4] = { originalCam->GetPosition()[0], originalCam->GetPosition()[1],
    originalCam->GetPosition()[2], 1.0 };
  reflectionMatrix->MultiplyPoint(pos, pos);

  // Reflect focal point
  double foc[4] = { originalCam->GetFocalPoint()[0], originalCam->GetFocalPoint()[1],
    originalCam->GetFocalPoint()[2], 1.0 };
  reflectionMatrix->MultiplyPoint(foc, foc);

  // Reflect up vector (direction, w=0)
  double up[4] = { originalCam->GetViewUp()[0], originalCam->GetViewUp()[1],
    originalCam->GetViewUp()[2], 0.0 };
  reflectionMatrix->MultiplyPoint(up, up);

  reflectedCam->DeepCopy(originalCam);
  reflectedCam->SetPosition(pos[0], pos[1], pos[2]);
  reflectedCam->SetFocalPoint(foc[0], foc[1], foc[2]);
  reflectedCam->SetViewUp(up[0], up[1], up[2]);
}
