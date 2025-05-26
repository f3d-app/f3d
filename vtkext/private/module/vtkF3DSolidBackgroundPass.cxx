#include "vtkF3DSolidBackgroundPass.h"

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

vtkStandardNewMacro(vtkF3DSolidBackgroundPass);

//------------------------------------------------------------------------------
void vtkF3DSolidBackgroundPass::Render(const vtkRenderState* state)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  vtkRenderer* ren = state->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  ren->Clear();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);

  assert(this->DelegatePass != nullptr);

  // create FBO and texture
  int pos[2];
  int size[2];
  ren->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

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
    state, size[0], size[1], size[0], size[1], this->FrameBufferObject, this->ColorTexture);
  renWin->GetState()->PopFramebufferBindings();

  if (!this->QuadHelper)
  {
    std::string FSSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Decl",
      "uniform sampler2D source;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(
      FSSource, "//VTK::FSQ::Impl", "gl_FragData[0] = texture(source, texCoord);");

    this->QuadHelper =
      std::make_shared<vtkOpenGLQuadHelper>(renWin, nullptr, FSSource.c_str(), nullptr);
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  assert(this->QuadHelper->Program && this->QuadHelper->Program->GetCompiled());

  this->ColorTexture->Activate();
  this->QuadHelper->Program->SetUniformi("source", this->ColorTexture->GetTextureUnit());

  // Enable blending with default VTK blending function
  // It is required since external window do not set it up
  ostate->vtkglEnable(GL_BLEND);
  ostate->vtkglBlendFuncSeparate(
    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglViewport(pos[0], pos[1], size[0], size[1]);
  ostate->vtkglScissor(pos[0], pos[1], size[0], size[1]);

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
void vtkF3DSolidBackgroundPass::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Superclass::ReleaseGraphicsResources(win);

  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(win);
  }
  if (this->ColorTexture)
  {
    this->ColorTexture->ReleaseGraphicsResources(win);
  }
}
