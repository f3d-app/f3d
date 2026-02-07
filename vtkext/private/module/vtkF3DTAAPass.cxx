#include "vtkF3DTAAPass.h"

#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DTAAPass);

//------------------------------------------------------------------------------
void vtkF3DTAAPass::Render(const vtkRenderState* state)
{
  vtkOpenGLClearErrorMacro();
  this->NumberOfRenderedProps = 0;

  vtkRenderer* renderer = state->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(renderer->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

  assert(this->DelegatePass != nullptr);

  // create framebuffer and textures
  int pos[2];
  renderer->GetTiledSizeAndOrigin(&this->viewPortSize[0], &this->viewPortSize[1], &pos[0], &pos[1]);

  if (this->HistoryTexture == nullptr)
  {
    this->HistoryTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->HistoryTexture->SetContext(renWin);
    this->HistoryTexture->SetFormat(GL_RGBA);
    this->HistoryTexture->SetInternalFormat(GL_RGBA32F);
    this->HistoryTexture->SetDataType(GL_FLOAT);
    this->HistoryTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->HistoryTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    renderer->GetTiledSizeAndOrigin(
      &this->viewPortSize[0], &this->viewPortSize[1], &pos[0], &pos[1]);
    this->HistoryTexture->Allocate2D(this->viewPortSize[0], this->viewPortSize[1], 4, VTK_FLOAT);
    this->ResetIterations();
  }
  this->HistoryTexture->Resize(this->viewPortSize[0], this->viewPortSize[1]);

  if (this->ColorTexture == nullptr)
  {
    this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorTexture->SetContext(renWin);
    this->ColorTexture->SetFormat(GL_RGBA);
    this->ColorTexture->SetInternalFormat(GL_RGBA32F);
    this->ColorTexture->SetDataType(GL_FLOAT);
    this->ColorTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorTexture->Allocate2D(this->viewPortSize[0], this->viewPortSize[1], 4, VTK_FLOAT);
  }
  this->ColorTexture->Resize(this->viewPortSize[0], this->viewPortSize[1]);

  if (this->MotionVectorTexture == nullptr)
  {
    this->MotionVectorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->MotionVectorTexture->SetContext(renWin);
    this->MotionVectorTexture->SetFormat(GL_RG);
    this->MotionVectorTexture->SetInternalFormat(GL_RG16F);
    this->MotionVectorTexture->SetDataType(GL_HALF_FLOAT);
    this->MotionVectorTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->MotionVectorTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->MotionVectorTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->MotionVectorTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->MotionVectorTexture->Allocate2D(
      this->viewPortSize[0], this->viewPortSize[1], 2, VTK_FLOAT);
  }
  this->MotionVectorTexture->Resize(this->viewPortSize[0], this->viewPortSize[1]);

  if (this->DepthTexture == nullptr)
  {
    this->DepthTexture = vtkTextureObject::New();
    this->DepthTexture->SetContext(renWin);
    this->DepthTexture->AllocateDepth(
      this->viewPortSize[0], this->viewPortSize[1], vtkTextureObject::Float32);
  }

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  if (!this->PreviousViewProjectionMatrix)
  {
    this->PreviousViewProjectionMatrix = vtkMatrix4x4::New();
  }
  if (!this->CurrentViewProjectionMatrix)
  {
    this->CurrentViewProjectionMatrix = vtkMatrix4x4::New();
    this->CurrentViewProjectionMatrix->Zero();
  }

  this->PreviousViewProjectionMatrix->DeepCopy(this->CurrentViewProjectionMatrix);

  vtkMatrix4x4* viewMat = renderer->GetActiveCamera()->GetViewTransformMatrix();
  vtkMatrix4x4* projMat = renderer->GetActiveCamera()->GetProjectionTransformMatrix(
    renderer->GetTiledAspectRatio(), -1.0, 1.0);
  vtkNew<vtkMatrix4x4> vpMat;
  vtkMatrix4x4::Multiply4x4(projMat, viewMat, vpMat);
  this->CurrentViewProjectionMatrix->DeepCopy(vpMat);

  this->ConfigureJitter(this->viewPortSize[0], this->viewPortSize[1]);
  this->ColorTexture->Activate();
  this->MotionVectorTexture->Activate();
  this->DepthTexture->Activate();

  this->PreRender(state);

  renWin->GetState()->PushFramebufferBindings();
  this->FrameBufferObject->Bind();
  this->FrameBufferObject->AddColorAttachment(0, this->ColorTexture);
  this->FrameBufferObject->AddColorAttachment(1, this->MotionVectorTexture);
  this->FrameBufferObject->ActivateDrawBuffers(2);
  this->FrameBufferObject->AddDepthAttachment(this->DepthTexture);

  this->DelegatePass->Render(state);
  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();

  this->FrameBufferObject->RemoveColorAttachments(2);
  this->FrameBufferObject->RemoveDepthAttachment();
  renWin->GetState()->PopFramebufferBindings();
  this->PostRender(state);

  if (!this->QuadHelper)
  {
    std::string TAAResolveFS = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();
    vtkShaderProgram::Substitute(TAAResolveFS, "//VTK::FSQ::Decl",
      "uniform sampler2D colorTexture;\n"
      "uniform sampler2D historyTexture;\n"
      "uniform sampler2D motionVectorTexture;\n"
      "uniform vec2 framebufferSize;"
      "uniform float blendFactor;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(TAAResolveFS, "//VTK::FSQ::Impl",
      "vec4 current = texture(colorTexture, texCoord);\n"
      "vec2 motion = texture(motionVectorTexture, texCoord).xy;\n"
      "vec4 history = texture(historyTexture, texCoord + motion / framebufferSize);\n"
      "vec2 texelSize = 1.0 / framebufferSize;\n"
      "vec4 c0 = texture(colorTexture, texCoord + vec2( texelSize.x, 0.0));\n"
      "vec4 c1 = texture(colorTexture, texCoord + vec2(-texelSize.x, 0.0));\n"
      "vec4 c2 = texture(colorTexture, texCoord + vec2(0.0,  texelSize.y));\n"
      "vec4 c3 = texture(colorTexture, texCoord + vec2(0.0, -texelSize.y));\n"
      "vec4 boxMin = min(current, min(c0, min(c1, min(c2, c3))));\n"
      "vec4 boxMax = max(current, max(c0, max(c1, max(c2, c3))));\n"
      "history = clamp(history, boxMin, boxMax);\n"
      "gl_FragData[0] = mix(current, history, blendFactor);\n"
      "//VTK::FSQ::Impl");
    this->QuadHelper =
      std::make_shared<vtkOpenGLQuadHelper>(renWin, nullptr, TAAResolveFS.c_str(), nullptr);
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  assert(this->QuadHelper->Program && this->QuadHelper->Program->GetCompiled());

  this->HistoryTexture->Activate();
  this->QuadHelper->Program->SetUniformi("colorTexture", this->ColorTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi("historyTexture", this->HistoryTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi(
    "motionVectorTexture", this->MotionVectorTexture->GetTextureUnit());
  float framebufferSize[2] = { static_cast<float>(this->viewPortSize[0]),
    static_cast<float>(this->viewPortSize[1]) };
  this->QuadHelper->Program->SetUniform2f("framebufferSize", framebufferSize);

  const float blendFactor = this->HistoryIteration / (this->HistoryIteration + 1.0f);
  this->QuadHelper->Program->SetUniformf("blendFactor", blendFactor);
  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglClear(GL_DEPTH_BUFFER_BIT);
  ostate->vtkglViewport(pos[0], pos[1], this->viewPortSize[0], this->viewPortSize[1]);
  ostate->vtkglScissor(pos[0], pos[1], this->viewPortSize[0], this->viewPortSize[1]);

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();
  this->HistoryTexture->Deactivate();
  this->MotionVectorTexture->Deactivate();
  this->DepthTexture->Deactivate();
  this->HistoryTexture->CopyFromFrameBuffer(pos[0], pos[1], this->viewPortSize[0],
    this->viewPortSize[1], this->viewPortSize[0], this->viewPortSize[1]);
  this->HistoryIteration = std::min(this->HistoryIteration + 1, 1024);

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
void vtkF3DTAAPass::ReleaseGraphicsResources(vtkWindow* window)
{
  this->Superclass::ReleaseGraphicsResources(window);
  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(window);
  }
  if (this->ColorTexture)
  {
    this->ColorTexture->ReleaseGraphicsResources(window);
  }
  if (this->HistoryTexture)
  {
    this->HistoryTexture->ReleaseGraphicsResources(window);
  }
  if (this->MotionVectorTexture)
  {
    this->MotionVectorTexture->ReleaseGraphicsResources(window);
  }
  if (this->DepthTexture)
  {
    this->DepthTexture->ReleaseGraphicsResources(window);
  }
}

//------------------------------------------------------------------------------
bool vtkF3DTAAPass::PreReplaceShaderValues(std::string& vertexShader,
  std::string& vtkNotUsed(geometryShader), std::string& fragmentShader, vtkAbstractMapper* mapper,
  vtkProp* vtkNotUsed(prop))
{
  if (vtkPolyDataMapper::SafeDownCast(mapper) != nullptr)
  {
    // jitter
    vtkShaderProgram::Substitute(vertexShader, "//VTK::Clip::Dec",
      "uniform vec2 jitter;\n"
      "//VTK::Clip::Dec",
      false);
    vtkShaderProgram::Substitute(vertexShader, "//VTK::Picking::Impl",
      "  gl_Position.xy += jitter * gl_Position.w;\n//VTK::Picking::Impl", false);

    // motion vector
    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Clip::Dec",
      "//VTK::Clip::Dec\n"
      "uniform vec2 framebufferSize;\n"
      "uniform mat4 currentVPInverse;\n"
      "uniform mat4 previousVP;\n",
      false);
    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Light::Impl",
      "//VTK::Light::Impl\n"
      "vec2 uv = gl_FragCoord.xy;\n"
      "float depth = gl_FragCoord.z;\n"
      "vec2 ndcXY = uv/framebufferSize * 2.0 - 1.0;\n"
      "float ndcZ = depth * 2.0 - 1.0;\n"
      "vec4 ndc = vec4(ndcXY, ndcZ, 1.0);\n"
      "vec4 worldPos = currentVPInverse * ndc;\n"
      "worldPos /= worldPos.w;"
      "vec4 prevClip = previousVP * vec4(worldPos.xyz, 1.0);\n"
      "prevClip /= prevClip.w;"
      "vec2 prevUV = (prevClip.xy * 0.5 + 0.5) * framebufferSize;"
      "vec2 motion = prevUV - uv;\n"
      "gl_FragData[1] = vec4(motion, 0, 1);\n",
      false);
  }
  return true;
}

//------------------------------------------------------------------------------
bool vtkF3DTAAPass::SetShaderParameters(vtkShaderProgram* program,
  vtkAbstractMapper* vtkNotUsed(mapper), vtkProp* vtkNotUsed(prop),
  vtkOpenGLVertexArrayObject* vtkNotUsed(VAO))
{
  program->SetUniform2f("jitter", this->Jitter);
  float size[2] = { static_cast<float>(this->viewPortSize[0]),
    static_cast<float>(this->viewPortSize[1]) };
  program->SetUniform2f("framebufferSize", size);

  vtkNew<vtkMatrix4x4> currentVPInverse;
  currentVPInverse->DeepCopy(this->CurrentViewProjectionMatrix);
  currentVPInverse->Transpose();
  currentVPInverse->Invert();

  vtkNew<vtkMatrix4x4> previousVP;
  previousVP->DeepCopy(this->PreviousViewProjectionMatrix);
  previousVP->Transpose();

  program->SetUniformMatrix("currentVPInverse", currentVPInverse);
  program->SetUniformMatrix("previousVP", previousVP);
  return true;
}

//------------------------------------------------------------------------------
void vtkF3DTAAPass::ConfigureJitter(int w, int h)
{
  Jitter[0] = this->ConfigureHaltonSequence(0);
  Jitter[1] = this->ConfigureHaltonSequence(1);

  Jitter[0] = ((Jitter[0] - 0.5f) / w) * 2.0f;
  Jitter[1] = ((Jitter[1] - 0.5f) / h) * 2.0f;
}

//------------------------------------------------------------------------------
float vtkF3DTAAPass::ConfigureHaltonSequence(int direction)
{
  assert(direction == 0 || direction == 1);

  int base = 2 + direction;
  int& numerator = this->TaaHaltonNumerator[direction];
  int& denominator = this->TaaHaltonDenominator[direction];

  int difference = denominator - numerator;
  if (difference == 1)
  {
    numerator = 1;
    denominator *= base;
  }
  else
  {
    int quotient = denominator / base;
    while (difference <= quotient && quotient > 0)
    {
      quotient = quotient / base;
    }

    numerator = (base + 1) * quotient - difference;
  }

  return static_cast<float>(numerator) / static_cast<float>(denominator);
}
