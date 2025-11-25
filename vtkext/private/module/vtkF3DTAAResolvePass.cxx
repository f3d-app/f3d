#include "vtkF3DTAAResolvePass.h"

#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DTAAResolvePass);

//------------------------------------------------------------------------------
void vtkF3DTAAResolvePass::Render(const vtkRenderState* state)
{
  vtkOpenGLClearErrorMacro();
  this->NumberOfRenderedProps = 0;

  vtkRenderer* renderer = state->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(renderer->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

  assert(this->DelegatePass != nullptr);

  this->PreRender(state);

  // create framebuffer and textures
  int pos[2];
  int size[2];
  renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

  if (this->HistoryTexture == nullptr)
  {
    this->HistoryTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->HistoryTexture->SetContext(renWin);
    this->HistoryTexture->SetFormat(GL_RGBA);
    this->HistoryTexture->SetInternalFormat(GL_RGBA16F);
    this->HistoryTexture->SetDataType(GL_HALF_FLOAT);
    this->HistoryTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->HistoryTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);
    this->HistoryTexture->Allocate2D(size[0], size[1], 4, VTK_FLOAT);
    this->ResetIterations();
  }
  this->HistoryTexture->Resize(size[0], size[1]);

  if (this->ColorTexture == nullptr)
  {
    this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorTexture->SetContext(renWin);
    this->ColorTexture->SetFormat(GL_RGBA);
    this->ColorTexture->SetInternalFormat(GL_RGBA16F);
    this->ColorTexture->SetDataType(GL_HALF_FLOAT);
    this->ColorTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorTexture->Allocate2D(size[0], size[1], 4, VTK_FLOAT);
  }
  this->ColorTexture->Resize(size[0], size[1]);

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
    this->MotionVectorTexture->Allocate2D(size[0], size[1], 2, VTK_FLOAT);
  }
  this->MotionVectorTexture->Resize(size[0], size[1]);

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  renWin->GetState()->PushFramebufferBindings();
  this->FrameBufferObject->Bind();
  this->FrameBufferObject->AddColorAttachment(0, this->ColorTexture);
  this->FrameBufferObject->AddColorAttachment(1, this->MotionVectorTexture);
  this->FrameBufferObject->ActivateDrawBuffers(2);

  this->DelegatePass->Render(state);
  renWin->GetState()->PopFramebufferBindings();

  this->PostRender(state);
  this->PreviousViewProjectionMatrix = renderer->GetActiveCamera()->GetViewTransformMatrix();

  if (!this->QuadHelper)
  {
    std::string TAAResolveFS = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();
    vtkShaderProgram::Substitute(TAAResolveFS, "//VTK::FSQ::Decl",
      "uniform sampler2D colorTexture;\n"
      "uniform sampler2D historyTexture;\n"
      "uniform float blendFactor;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(TAAResolveFS, "//VTK::FSQ::Impl",
      "vec4 current = texture(colorTexture, texCoord);\n"
      "vec4 history = texture(historyTexture, texCoord);\n"
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
  this->ColorTexture->Activate();
  this->QuadHelper->Program->SetUniformi("colorTexture", this->ColorTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi("historyTexture", this->HistoryTexture->GetTextureUnit());

  const float blendFactor = this->HistoryIteration / (this->HistoryIteration + 1.0f);
  this->QuadHelper->Program->SetUniformf("blendFactor", blendFactor);
  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglClear(GL_DEPTH_BUFFER_BIT);
  ostate->vtkglViewport(pos[0], pos[1], size[0], size[1]);
  ostate->vtkglScissor(pos[0], pos[1], size[0], size[1]);

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();
  this->HistoryTexture->Deactivate();
  this->HistoryTexture->CopyFromFrameBuffer(pos[0], pos[1], size[0], size[1], size[0], size[1]);
  this->HistoryIteration = std::min(this->HistoryIteration + 1, 1024);

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
bool vtkF3DTAAResolvePass::PreReplaceShaderValues(std::string& vertexShader,
  std::string& geometryShader, std::string& fragmentShader, vtkAbstractMapper* mapper,
  vtkProp* prop)
{
  if (vtkPolyDataMapper::SafeDownCast(mapper) != nullptr && this->PreviousViewProjectionMatrix)
  {
    vtkShaderProgram::Substitute(vertexShader, "//VTK::Camera::Impl",
      "//VTK::Camera::Impl\nTAA_PrevClipPos = TAA_PreviousVP * vertexMC;\n", false);

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::System::Dec",
      "//VTK::System::Dec\nin vec4 TAA_PrevClipPos;\nuniform sampler2D colorTexture;", false);

    vtkShaderProgram::Substitute(fragmentShader, "//VTK::Light::Impl",
      "//VTK::Light::Impl\n"
      "vec2 currNDC = gl_FragCoord.xy; // will normalize later in resolve if needed\n"
      "vec4 prevClip = TAA_PrevClipPos;\n"
      "prevClip /= prevClip.w;\n"
      "vec2 prevNDC = prevClip.xy * 0.5 + 0.5;\n"
      "vec2 motion = prevNDC - (currNDC / vec2(textureSize(colorTexture,0))); // screen-space "
      "motion\n"
      "gl_FragData[1] = vec4(motion, 0.0, 1.0);\n",
      false);
  }
  return true;
}

//------------------------------------------------------------------------------
bool vtkF3DTAAResolvePass::SetShaderParameters(vtkShaderProgram* program, vtkAbstractMapper* mapper,
  vtkProp* prop, vtkOpenGLVertexArrayObject* VAO)
{
  if (this->PreviousViewProjectionMatrix == nullptr)
  {
    return true;
  }

  program->SetUniformMatrix("TAA_PreviousVP", this->PreviousViewProjectionMatrix);
  program->SetUniformi("colorTexture", this->ColorTexture->GetTextureUnit());
  return true;
}

//------------------------------------------------------------------------------
void vtkF3DTAAResolvePass::ReleaseGraphicsResources(vtkWindow* window)
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
}
