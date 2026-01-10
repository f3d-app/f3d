#include "vtkF3DTAAPass.h"

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

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  this->ConfigureJitter(size[0], size[1]);
  this->PreRender(state);
  renWin->GetState()->PushFramebufferBindings();
  this->RenderDelegate(
    state, size[0], size[1], size[0], size[1], this->FrameBufferObject, this->ColorTexture);
  renWin->GetState()->PopFramebufferBindings();
  this->PostRender(state);

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
}

//------------------------------------------------------------------------------
bool vtkF3DTAAPass::PreReplaceShaderValues(std::string& vertexShader,
  std::string& vtkNotUsed(geometryShader), std::string& vtkNotUsed(fragmentShader),
  vtkAbstractMapper* mapper, vtkProp* vtkNotUsed(prop))
{
  if (vtkPolyDataMapper::SafeDownCast(mapper) != nullptr)
  {
    vtkShaderProgram::Substitute(
      vertexShader, "//VTK::Clip::Dec", "uniform vec2 jitter;\n//VTK::Clip::Dec", false);
    vtkShaderProgram::Substitute(
      vertexShader, "//VTK::CustomEnd::Impl", "  gl_Position.xy += jitter;\n", false);
  }
  return true;
}

//------------------------------------------------------------------------------
bool vtkF3DTAAPass::SetShaderParameters(vtkShaderProgram* program,
  vtkAbstractMapper* vtkNotUsed(mapper), vtkProp* vtkNotUsed(prop),
  vtkOpenGLVertexArrayObject* vtkNotUsed(VAO))
{
  program->SetUniform2f("jitter", this->Jitter);
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
