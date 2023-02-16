#include "vtkF3DHexagonalBokehBlurPass.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderUtilities.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkOpenGLVertexArrayObject.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"

vtkStandardNewMacro(vtkF3DHexagonalBokehBlurPass);

constexpr std::string_view BlurFunc()
{
  // clang-format off
  return R"#(
const float PI = 3.14159265359;

vec4 BlurTexture(sampler2D tex, vec2 uv, vec2 direction)
{
  vec4 color = vec4(0.0f);
  float acc = 0.0;

  uv += 0.5 * invViewDims * direction;

  const float step = 0.05;

  for (float i = 0.0; i < 1.0; i += step)
  {
    vec2 offset = ((i * coc) * direction) * invViewDims;
    color += texture(tex, uv + offset);
    acc += 1.0;
  }

  return color / acc;
})#";
  // clang-format on
}

//------------------------------------------------------------------------------
vtkF3DHexagonalBokehBlurPass::vtkF3DHexagonalBokehBlurPass() = default;

//------------------------------------------------------------------------------
vtkF3DHexagonalBokehBlurPass::~vtkF3DHexagonalBokehBlurPass()
{
  if (this->FrameBufferObject != nullptr)
  {
    vtkErrorMacro(<< "FrameBufferObject should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->VerticalBlurTexture != nullptr)
  {
    vtkErrorMacro(<< "VerticalBlur should have been deleted in ReleaseGraphicsResources().");
  }
  if (this->DiagonalBlurTexture != nullptr)
  {
    vtkErrorMacro(<< "DiagonalBlur should have been deleted in ReleaseGraphicsResources().");
  }
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "CircleOfConfusion: " << this->CircleOfConfusion << std::endl;
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::InitializeGraphicsResources(
  vtkOpenGLRenderWindow* renWin, int w, int h)
{
  if (this->BackgroundTexture == nullptr)
  {
    this->BackgroundTexture = vtkTextureObject::New();
    this->BackgroundTexture->SetContext(renWin);
    this->BackgroundTexture->SetFormat(GL_RGBA);
    this->BackgroundTexture->SetInternalFormat(GL_RGBA32F);
    this->BackgroundTexture->SetDataType(GL_FLOAT);
    this->BackgroundTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->BackgroundTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->BackgroundTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->BackgroundTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->BackgroundTexture->Allocate2D(w, h, 4, VTK_FLOAT);
  }

  if (this->VerticalBlurTexture == nullptr)
  {
    this->VerticalBlurTexture = vtkTextureObject::New();
    this->VerticalBlurTexture->SetContext(renWin);
    this->VerticalBlurTexture->SetFormat(GL_RGBA);
    this->VerticalBlurTexture->SetInternalFormat(GL_RGBA32F);
    this->VerticalBlurTexture->SetDataType(GL_FLOAT);
    this->VerticalBlurTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->VerticalBlurTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->VerticalBlurTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->VerticalBlurTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->VerticalBlurTexture->Allocate2D(w, h, 4, VTK_FLOAT);
  }

  if (this->DiagonalBlurTexture == nullptr)
  {
    this->DiagonalBlurTexture = vtkTextureObject::New();
    this->DiagonalBlurTexture->SetContext(renWin);
    this->DiagonalBlurTexture->SetFormat(GL_RGBA);
    this->DiagonalBlurTexture->SetInternalFormat(GL_RGBA32F);
    this->DiagonalBlurTexture->SetDataType(GL_FLOAT);
    this->DiagonalBlurTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->DiagonalBlurTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->DiagonalBlurTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->DiagonalBlurTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->DiagonalBlurTexture->Allocate2D(w, h, 4, VTK_FLOAT);
  }

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkOpenGLFramebufferObject::New();
    this->FrameBufferObject->SetContext(renWin);
  }
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::RenderDelegate(const vtkRenderState* s, int w, int h)
{
  this->PreRender(s);

  this->FrameBufferObject->GetContext()->GetState()->PushFramebufferBindings();
  this->FrameBufferObject->Bind();

  this->FrameBufferObject->AddColorAttachment(0, this->BackgroundTexture);
  this->FrameBufferObject->ActivateDrawBuffers(1);
  this->FrameBufferObject->StartNonOrtho(w, h);

  vtkOpenGLRenderer* glRen = vtkOpenGLRenderer::SafeDownCast(s->GetRenderer());

  glRen->GetState()->vtkglClear(GL_COLOR_BUFFER_BIT);

  this->DelegatePass->Render(s);
  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();

  this->FrameBufferObject->RemoveColorAttachments(1);

  this->FrameBufferObject->GetContext()->GetState()->PopFramebufferBindings();

  this->PostRender(s);
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::RenderDirectionalBlur(
  vtkOpenGLRenderWindow* renWin, int w, int h)
{
  if (this->BlurQuadHelper && this->BlurQuadHelper->ShaderChangeValue < this->GetMTime())
  {
    delete this->BlurQuadHelper;
    this->BlurQuadHelper = nullptr;
  }

  if (!this->BlurQuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    std::stringstream ssDecl;
    ssDecl << "uniform sampler2D backgroundTexture;\n";
    ssDecl << "uniform vec2 invViewDims;\n";
    ssDecl << "uniform float coc;\n";
    ssDecl << BlurFunc();
    ssDecl << "//VTK::FSQ::Decl";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", ssDecl.str());

    std::stringstream ssImpl;

    ssImpl << "  vec2 blurDir = vec2(cos(PI/2), sin(PI/2));\n";
    ssImpl << "  vec4 color1 = BlurTexture(backgroundTexture, texCoord, blurDir);\n";
    ssImpl << "  blurDir = vec2(cos(-PI/6), sin(-PI/6));\n";
    ssImpl << "  vec4 color2 = BlurTexture(backgroundTexture, texCoord, blurDir);\n";
    ssImpl << "  gl_FragData[0] = color1;\n";
    ssImpl << "  gl_FragData[1] = (color1 + color2) * 0.5;\n";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl", ssImpl.str());

    this->BlurQuadHelper = new vtkOpenGLQuadHelper(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), FSSource.c_str(), "");

    this->BlurQuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->BlurQuadHelper->Program);
  }

  if (!this->BlurQuadHelper->Program || !this->BlurQuadHelper->Program->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the Blur shader program.");
    return;
  }

  this->BackgroundTexture->Activate();
  this->BlurQuadHelper->Program->SetUniformi(
    "backgroundTexture", this->BackgroundTexture->GetTextureUnit());

  float invViewDims[2] = { 1.f / static_cast<float>(w), 1.f / static_cast<float>(h) };
  this->BlurQuadHelper->Program->SetUniform2f("invViewDims", invViewDims);

  this->BlurQuadHelper->Program->SetUniformf("coc", this->CircleOfConfusion);

  this->FrameBufferObject->GetContext()->GetState()->PushFramebufferBindings();
  this->FrameBufferObject->Bind();

  this->FrameBufferObject->AddColorAttachment(0, this->VerticalBlurTexture);
  this->FrameBufferObject->AddColorAttachment(1, this->DiagonalBlurTexture);
  this->FrameBufferObject->ActivateDrawBuffers(2);
  this->FrameBufferObject->StartNonOrtho(w, h);

  this->BlurQuadHelper->Render();

  this->FrameBufferObject->RemoveColorAttachments(2);

  this->FrameBufferObject->GetContext()->GetState()->PopFramebufferBindings();

  this->BackgroundTexture->Deactivate();
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::RenderRhomboidBlur(vtkOpenGLRenderWindow* renWin, int w, int h)
{
  if (this->RhomboidQuadHelper && this->RhomboidQuadHelper->ShaderChangeValue < this->GetMTime())
  {
    delete this->RhomboidQuadHelper;
    this->RhomboidQuadHelper = nullptr;
  }

  if (!this->RhomboidQuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    std::stringstream ssDecl;
    ssDecl << "uniform sampler2D verticalBlurTexture;\n";
    ssDecl << "uniform sampler2D diagonalBlurTexture;\n";
    ssDecl << "uniform vec2 invViewDims;\n";
    ssDecl << "uniform float coc;\n";
    ssDecl << BlurFunc();
    ssDecl << "//VTK::FSQ::Decl";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", ssDecl.str());

    std::stringstream ssImpl;

    ssImpl << "  vec2 blurDir = vec2(cos(-PI/6), sin(-PI/6));\n";
    ssImpl << "  vec4 color1 = BlurTexture(verticalBlurTexture, texCoord, blurDir);\n";
    ssImpl << "  blurDir = vec2(cos(-5*PI/6), sin(-5*PI/6));\n";
    ssImpl << "  vec4 color2 = BlurTexture(diagonalBlurTexture, texCoord, blurDir);\n";
    ssImpl << "  gl_FragData[0] = (color1 + color2) * 0.5f;\n";

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl", ssImpl.str());

    this->RhomboidQuadHelper = new vtkOpenGLQuadHelper(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), FSSource.c_str(), "");

    this->RhomboidQuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->RhomboidQuadHelper->Program);
  }

  if (!this->RhomboidQuadHelper->Program || !this->RhomboidQuadHelper->Program->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the Rhomboid Blur shader program.");
    return;
  }

  this->VerticalBlurTexture->Activate();
  this->DiagonalBlurTexture->Activate();
  this->RhomboidQuadHelper->Program->SetUniformi(
    "verticalBlurTexture", this->VerticalBlurTexture->GetTextureUnit());
  this->RhomboidQuadHelper->Program->SetUniformi(
    "diagonalBlurTexture", this->DiagonalBlurTexture->GetTextureUnit());

  float invViewDims[2] = { 1.f / static_cast<float>(w), 1.f / static_cast<float>(h) };
  this->RhomboidQuadHelper->Program->SetUniform2f("invViewDims", invViewDims);

  this->RhomboidQuadHelper->Program->SetUniformf("coc", this->CircleOfConfusion);

  this->RhomboidQuadHelper->Render();

  this->BackgroundTexture->Deactivate();
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

  if (this->DelegatePass == nullptr)
  {
    vtkWarningMacro("no delegate in vtkF3DHexagonalBokehBlurPass.");
    return;
  }

  // create FBO and texture
  int x = 0, y = 0, w, h;
  vtkFrameBufferObjectBase* fbo = s->GetFrameBuffer();
  if (fbo)
  {
    fbo->GetLastSize(w, h);
  }
  else
  {
    r->GetTiledSizeAndOrigin(&w, &h, &x, &y);
  }

  this->InitializeGraphicsResources(renWin, w, h);

  this->BackgroundTexture->Resize(w, h);
  this->VerticalBlurTexture->Resize(w, h);
  this->DiagonalBlurTexture->Resize(w, h);

  ostate->vtkglViewport(x, y, w, h);
  ostate->vtkglScissor(x, y, w, h);

  this->RenderDelegate(s, w, h);

  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);

  this->RenderDirectionalBlur(renWin, w, h);
  this->RenderRhomboidBlur(renWin, w, h);

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
void vtkF3DHexagonalBokehBlurPass::ReleaseGraphicsResources(vtkWindow* w)
{
  assert("pre: w_exists" && w != nullptr);

  this->Superclass::ReleaseGraphicsResources(w);

  if (this->BlurQuadHelper != nullptr)
  {
    this->BlurQuadHelper->ReleaseGraphicsResources(w);
    delete this->BlurQuadHelper;
    this->BlurQuadHelper = nullptr;
  }
  if (this->RhomboidQuadHelper != nullptr)
  {
    this->RhomboidQuadHelper->ReleaseGraphicsResources(w);
    delete this->RhomboidQuadHelper;
    this->RhomboidQuadHelper = nullptr;
  }
  if (this->FrameBufferObject != nullptr)
  {
    this->FrameBufferObject->Delete();
    this->FrameBufferObject = nullptr;
  }
  if (this->DiagonalBlurTexture != nullptr)
  {
    this->DiagonalBlurTexture->Delete();
    this->DiagonalBlurTexture = nullptr;
  }
  if (this->VerticalBlurTexture != nullptr)
  {
    this->VerticalBlurTexture->Delete();
    this->VerticalBlurTexture = nullptr;
  }
  if (this->BackgroundTexture != nullptr)
  {
    this->BackgroundTexture->Delete();
    this->BackgroundTexture = nullptr;
  }
}
