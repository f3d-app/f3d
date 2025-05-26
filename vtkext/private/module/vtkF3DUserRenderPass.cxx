#include "vtkF3DUserRenderPass.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderUtilities.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"

vtkStandardNewMacro(vtkF3DUserRenderPass);

//------------------------------------------------------------------------------
void vtkF3DUserRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

  assert(this->DelegatePass != nullptr);

  // create FBO and texture
  int pos[2];
  int size[2];
  r->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

  if (this->ColorTexture == nullptr)
  {
    this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorTexture->SetContext(renWin);
    this->ColorTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorTexture->Allocate2D(size[0], size[1], 4, VTK_UNSIGNED_CHAR);
  }
  this->ColorTexture->Resize(size[0], size[1]);

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  renWin->GetState()->PushFramebufferBindings();
  this->RenderDelegate(
    s, size[0], size[1], size[0], size[1], this->FrameBufferObject, this->ColorTexture);
  renWin->GetState()->PopFramebufferBindings();

  if (this->QuadHelper && this->QuadHelper->ShaderChangeValue < this->GetMTime())
  {
    this->QuadHelper = nullptr;
  }

  if (!this->QuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl",
      "uniform sampler2D source;\n"
      "uniform ivec2 resolution;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl", this->UserShader);

    // Apply user shader
    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl", "gl_FragData[0] = pixel(texCoord);");

    this->QuadHelper =
      std::make_shared<vtkOpenGLQuadHelper>(renWin, nullptr, FSSource.c_str(), nullptr);
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  if (!this->QuadHelper->Program || !this->QuadHelper->Program->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the shader program.");
    return;
  }

  this->ColorTexture->Activate();
  this->QuadHelper->Program->SetUniformi("source", this->ColorTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniform2i("resolution", size);

  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglClear(GL_DEPTH_BUFFER_BIT);
  ostate->vtkglViewport(pos[0], pos[1], size[0], size[1]);
  ostate->vtkglScissor(pos[0], pos[1], size[0], size[1]);

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
void vtkF3DUserRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(w);
  }
  if (this->ColorTexture)
  {
    this->ColorTexture->ReleaseGraphicsResources(w);
  }
}
