#include "vtkF3DFrameCapture.h"

#include <vtkObjectFactory.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkPixelBufferObject.h>
#include <vtkShaderProgram.h>

//----------------------------------------------------------------------------
vtkObjectFactoryNewMacro(vtkF3DFrameCapture);

struct vtkF3DFrameCapture::Internals
{
  // Resources for YUV conversion
  std::unique_ptr<vtkOpenGLQuadHelper> QuadHelperY;
  std::unique_ptr<vtkOpenGLQuadHelper> QuadHelperUV;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FboY;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FboUV;
  vtkSmartPointer<vtkTextureObject> TextureY;
  vtkSmartPointer<vtkTextureObject> TextureU;
  vtkSmartPointer<vtkTextureObject> TextureV;
};

//----------------------------------------------------------------------------
vtkF3DFrameCapture::vtkF3DFrameCapture()
  : Pimpl(new Internals())
{
}

//----------------------------------------------------------------------------
vtkF3DFrameCapture::~vtkF3DFrameCapture() = default;

//----------------------------------------------------------------------------
void vtkF3DFrameCapture::RenderYUV(vtkOpenGLRenderer* ren)
{
  this->InputTexture->SetMinificationFilter(vtkTextureObject::Linear);
  this->InputTexture->SetMagnificationFilter(vtkTextureObject::Linear);

  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());

  if (this->Pimpl->TextureY == nullptr)
  {
    this->Pimpl->TextureY = vtkSmartPointer<vtkTextureObject>::New();
    this->Pimpl->TextureY->SetContext(renWin);
    this->Pimpl->TextureY->SetFormat(GL_RED);
    this->Pimpl->TextureY->SetInternalFormat(GL_R8);
    this->Pimpl->TextureY->SetDataType(GL_UNSIGNED_BYTE);
    this->Pimpl->TextureY->Allocate2D(
      this->InputTexture->GetWidth(), this->InputTexture->GetHeight(), 1, VTK_UNSIGNED_CHAR);
  }

  if (this->Pimpl->TextureU == nullptr)
  {
    this->Pimpl->TextureU = vtkSmartPointer<vtkTextureObject>::New();
    this->Pimpl->TextureU->SetContext(renWin);
    this->Pimpl->TextureU->SetFormat(GL_RED);
    this->Pimpl->TextureU->SetInternalFormat(GL_R8);
    this->Pimpl->TextureU->SetDataType(GL_UNSIGNED_BYTE);
    this->Pimpl->TextureU->Allocate2D(this->InputTexture->GetWidth() / 2,
      this->InputTexture->GetHeight() / 2, 1, VTK_UNSIGNED_CHAR);
  }

  if (this->Pimpl->TextureV == nullptr)
  {
    this->Pimpl->TextureV = vtkSmartPointer<vtkTextureObject>::New();
    this->Pimpl->TextureV->SetContext(renWin);
    this->Pimpl->TextureV->SetFormat(GL_RED);
    this->Pimpl->TextureV->SetInternalFormat(GL_R8);
    this->Pimpl->TextureV->SetDataType(GL_UNSIGNED_BYTE);
    this->Pimpl->TextureV->Allocate2D(this->InputTexture->GetWidth() / 2,
      this->InputTexture->GetHeight() / 2, 1, VTK_UNSIGNED_CHAR);
  }

  if (this->Pimpl->FboY == nullptr)
  {
    this->Pimpl->FboY = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->Pimpl->FboY->SetContext(renWin);
  }

  if (this->Pimpl->FboUV == nullptr)
  {
    this->Pimpl->FboUV = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->Pimpl->FboUV->SetContext(renWin);
  }

  renWin->GetState()->PushFramebufferBindings();
  this->InputTexture->Activate();

  // RGB to YUV conversion
  // See https://en.wikipedia.org/wiki/Y%E2%80%B2UV

  // Luma step (render Y plane)
  if (!this->Pimpl->QuadHelperY)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl",
      "uniform sampler2D source;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl",
      "vec3 color = texture(source, vec2(texCoord.x, 1.0 - texCoord.y)).rgb;\n"
      "float Y = 0.257 * color.r + 0.504 * color.g + 0.098 * color.b + 16.0 / 255.0;\n"
      "gl_FragData[0] = vec4(Y, 0.0, 0.0, 1.0);\n");

    this->Pimpl->QuadHelperY =
      std::make_unique<vtkOpenGLQuadHelper>(renWin, nullptr, FSSource.c_str(), nullptr);
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->Pimpl->QuadHelperY->Program);
  }

  this->Pimpl->QuadHelperY->Program->SetUniformi("source", this->InputTexture->GetTextureUnit());
  this->Pimpl->FboY->Bind();
  this->Pimpl->FboY->AddColorAttachment(0, this->Pimpl->TextureY);
  this->Pimpl->FboY->StartNonOrtho(this->InputTexture->GetWidth(), this->InputTexture->GetHeight());
  this->Pimpl->QuadHelperY->Render();

  // Chroma step (render U and V planes in a single pass)
  if (!this->Pimpl->QuadHelperUV)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl",
      "uniform sampler2D source;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl",
      "ivec2 srcSize = textureSize(source, 0);\n"
      "vec2 uv = 2.0 * (floor(texCoord * vec2(srcSize) * 0.5) * 2.0 + 1.0) / vec2(srcSize);\n"
      "vec3 color = texture(source, vec2(uv.x, 1.0 - uv.y)).rgb;\n"
      "float U = -0.148 * color.r - 0.291 * color.g + 0.439 * color.b + 128.0 / 255.0;\n"
      "float V = 0.439 * color.r - 0.368 * color.g - 0.071 * color.b + 128.0 / 255.0;\n"
      "gl_FragData[0] = vec4(U, 0.0, 0.0, 1.0);\n"
      "gl_FragData[1] = vec4(V, 0.0, 0.0, 1.0);\n");

    this->Pimpl->QuadHelperUV =
      std::make_unique<vtkOpenGLQuadHelper>(renWin, nullptr, FSSource.c_str(), nullptr);
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->Pimpl->QuadHelperUV->Program);
  }

  this->Pimpl->QuadHelperUV->Program->SetUniformi("source", this->InputTexture->GetTextureUnit());
  this->Pimpl->FboUV->Bind();
  this->Pimpl->FboUV->AddColorAttachment(0, this->Pimpl->TextureU);
  this->Pimpl->FboUV->AddColorAttachment(1, this->Pimpl->TextureV);
  this->Pimpl->FboUV->ActivateDrawBuffers(2);
  this->Pimpl->FboUV->StartNonOrtho(
    this->InputTexture->GetWidth() / 2, this->InputTexture->GetHeight() / 2);
  this->Pimpl->QuadHelperUV->Render();

  this->InputTexture->Deactivate();
  renWin->GetState()->PopFramebufferBindings();
}

//----------------------------------------------------------------------------
void vtkF3DFrameCapture::Capture(
  vtkOpenGLRenderer* ren, std::byte* yPlane, std::byte* uPlane, std::byte* vPlane)
{
  assert(this->InputTexture != nullptr);

  // Check if the input texture size has changed, and release resources if so
  if (this->Pimpl->FboY != nullptr &&
    (this->Pimpl->FboY->GetLastSize()[0] != static_cast<int>(this->InputTexture->GetWidth()) ||
      this->Pimpl->FboY->GetLastSize()[1] != static_cast<int>(this->InputTexture->GetHeight())))
  {
    this->ReleaseGraphicsResources(ren->GetRenderWindow());
  }

  // convert to YUV and store in Pimpl->TextureY and Pimpl->TextureUV
  this->RenderYUV(ren);

  // read Y and UV planes
  auto downloadY = vtkSmartPointer<vtkPixelBufferObject>::Take(this->Pimpl->TextureY->Download());
  auto downloadU = vtkSmartPointer<vtkPixelBufferObject>::Take(this->Pimpl->TextureU->Download());
  auto downloadV = vtkSmartPointer<vtkPixelBufferObject>::Take(this->Pimpl->TextureV->Download());

  vtkIdType increments[2] = { 0, 0 }; // contiguous memory layout

  unsigned int dimsY[2] = { this->InputTexture->GetWidth(), this->InputTexture->GetHeight() };
  downloadY->Download2D(VTK_UNSIGNED_CHAR, yPlane, dimsY, 1, increments);

  unsigned int dimsUV[2] = { this->InputTexture->GetWidth() / 2,
    this->InputTexture->GetHeight() / 2 };
  downloadU->Download2D(VTK_UNSIGNED_CHAR, uPlane, dimsUV, 1, increments);
  downloadV->Download2D(VTK_UNSIGNED_CHAR, vPlane, dimsUV, 1, increments);
}

//------------------------------------------------------------------------------
void vtkF3DFrameCapture::ReleaseGraphicsResources(vtkWindow* win)
{
  if (this->Pimpl->QuadHelperY)
  {
    this->Pimpl->QuadHelperY->ReleaseGraphicsResources(win);
    this->Pimpl->QuadHelperY = nullptr;
  }
  if (this->Pimpl->QuadHelperUV)
  {
    this->Pimpl->QuadHelperUV->ReleaseGraphicsResources(win);
    this->Pimpl->QuadHelperUV = nullptr;
  }
  if (this->Pimpl->FboY)
  {
    this->Pimpl->FboY->ReleaseGraphicsResources(win);
    this->Pimpl->FboY = nullptr;
  }
  if (this->Pimpl->FboUV)
  {
    this->Pimpl->FboUV->ReleaseGraphicsResources(win);
    this->Pimpl->FboUV = nullptr;
  }
  if (this->Pimpl->TextureY)
  {
    this->Pimpl->TextureY->ReleaseGraphicsResources(win);
    this->Pimpl->TextureY = nullptr;
  }
  if (this->Pimpl->TextureU)
  {
    this->Pimpl->TextureU->ReleaseGraphicsResources(win);
    this->Pimpl->TextureU = nullptr;
  }
  if (this->Pimpl->TextureV)
  {
    this->Pimpl->TextureV->ReleaseGraphicsResources(win);
    this->Pimpl->TextureV = nullptr;
  }
}
