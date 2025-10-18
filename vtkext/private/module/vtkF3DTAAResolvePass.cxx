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

  this->InspectCameraMovement(renderer);

  // create framebuffer and textures
  int pos[2];
  int size[2];
  renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

  if (this->HistoryTexture == nullptr)
  {
    this->HistoryTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->HistoryTexture->SetContext(renWin);
    this->HistoryTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->HistoryTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->HistoryTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);
    this->HistoryTexture->Allocate2D(size[0], size[1], 4, VTK_FLOAT);
    this->HistoryInitialized = false;
  }
  this->HistoryTexture->Resize(size[0], size[1]);

  if (this->ColorTexture == nullptr)
  {
    this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorTexture->SetContext(renWin);
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

  renWin->GetState()->PushFramebufferBindings();
  this->RenderDelegate(
    state, size[0], size[1], size[0], size[1], this->FrameBufferObject, this->ColorTexture);
  renWin->GetState()->PopFramebufferBindings();

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

  const float blendFactor = this->HistoryInitialized ? 0.9f : 0.0f;
  this->QuadHelper->Program->SetUniformf("blendFactor", blendFactor);
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
  this->HistoryInitialized = true;

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//----------------------------------------------------------------------------
void vtkF3DTAAResolvePass::InspectCameraMovement(vtkRenderer* renderer)
{
  vtkCamera* cam = renderer->GetActiveCamera();

  assert(cam != nullptr);

  double position[3];
  cam->GetPosition(position);
  double focalPoint[3];
  cam->GetFocalPoint(focalPoint);
  double viewUp[3];
  cam->GetViewUp(viewUp);

  if (this->LastPosition[0] != position[0] || this->LastPosition[1] != position[1] ||
    this->LastPosition[2] != position[2] || this->LastFocalPoint[0] != focalPoint[0] ||
    this->LastFocalPoint[1] != focalPoint[1] || this->LastFocalPoint[2] != focalPoint[2] ||
    this->LastViewUp[0] != viewUp[0] || this->LastViewUp[1] != viewUp[1] ||
    this->LastViewUp[2] != viewUp[2])
  {
    this->HistoryInitialized = false;

    this->LastPosition[0] = position[0];
    this->LastPosition[1] = position[1];
    this->LastPosition[2] = position[2];

    this->LastFocalPoint[0] = focalPoint[0];
    this->LastFocalPoint[1] = focalPoint[1];
    this->LastFocalPoint[2] = focalPoint[2];

    this->LastViewUp[0] = viewUp[0];
    this->LastViewUp[1] = viewUp[1];
    this->LastViewUp[2] = viewUp[2];
  }
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
