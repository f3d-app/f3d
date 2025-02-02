#include "vtkF3DOverlayRenderPass.h"

#include <vtkCameraPass.h>
#include <vtkDefaultPass.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include "vtkOpenGLRenderUtilities.h"
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkRenderState.h>
#include <vtkShaderProgram.h>
#include <vtkSkybox.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DOverlayRenderPass);

void vtkF3DOverlayRenderPass::Render(const vtkRenderState* s)
{
  this->Initialize(s);

  vtkRenderState overlayState(s->GetRenderer());
  overlayState.SetPropArrayAndCount(
    this->OverlayProps.data(), static_cast<int>(this->OverlayProps.size()));
  overlayState.SetFrameBuffer(s->GetFrameBuffer());

  this->OverlayPass->Render(&overlayState);

  //blend with current buffer here
  this->CompositeOverlay(s);
}

void vtkF3DOverlayRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->OverlayPass)
  {
    this->OverlayPass->ReleaseGraphicsResources(w);
  }
  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(w);
  }
}

void vtkF3DOverlayRenderPass::Initialize(const vtkRenderState* s)
{
  this->OverlayProps.clear();

  //assign props to the overlay pass
  vtkProp** props = s->GetPropArray();
  for (int i = 0; i < s->GetPropArrayCount(); i++)
  {
    vtkProp* prop = props[i];
    if (!vtkSkybox::SafeDownCast(prop) && !vtkProp3D::SafeDownCast(prop))
    {
      this->OverlayProps.push_back(prop);
    }
  }

  this->ReleaseGraphicsResources(s->GetRenderer()->GetRenderWindow());

  vtkNew<vtkDefaultPass> overlayP;
  vtkNew<vtkCameraPass> overlayCamP;
  overlayCamP->SetDelegatePass(overlayP);

  this->OverlayPass = vtkSmartPointer<vtkFramebufferPass>::New();
  this->OverlayPass->SetDelegatePass(overlayCamP);
  this->OverlayPass->SetColorFormat(vtkTextureObject::Float32);
}

void vtkF3DOverlayRenderPass::CompositeOverlay(const vtkRenderState* s)
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
      "uniform sampler2D texOverlay;\n"
      "vec3 toLinear(vec3 color) { return pow(color.rgb, vec3(2.2)); }\n"
      "vec3 toSRGB(vec3 color) { return pow(color.rgb, vec3(1.0 / 2.2)); }\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl", 
      "  vec4 ovlSample = texture(texOverlay, texCoord);\n"
      "  if (ovlSample.a > 0.0)\n"
      "    ovlSample.rgb = toLinear(ovlSample.rgb / ovlSample.a);\n"
      "  ovlSample.rgb *= ovlSample.a;\n"
      "//VTK::FSQ::Impl");

    vtkShaderProgram::Substitute(FSSource, "//VTK::FSQ::Impl",
      "  vec4 result = texture(source, texCoord);\n"
      "  result.rgb = (1.0 - ovlSample.a) * result.rgb + ovlSample.rgb;\n"
      "  result.a = (1.0 - ovlSample.a) * result.a + ovlSample.a;\n"
      "  gl_FragData[0] = result;\n"
      "//VTK::FSQ::Impl");

    this->QuadHelper =
      std::make_shared<vtkOpenGLQuadHelper>(renWin, nullptr, FSSource.c_str(), nullptr);
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  this->OverlayPass->GetColorTexture()->Activate();
  this->ColorTexture->Activate();

  this->QuadHelper->Program->SetUniformi(
    "texOverlay", this->OverlayPass->GetColorTexture()->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi("source", this->ColorTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniform2i("resolution", size);
  
  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglClear(GL_DEPTH_BUFFER_BIT);
  ostate->vtkglViewport(pos[0], pos[1], size[0], size[1]);
  ostate->vtkglScissor(pos[0], pos[1], size[0], size[1]);

  this->QuadHelper->Render();

  this->OverlayPass->GetColorTexture()->Deactivate();
  this->ColorTexture->Deactivate();

  vtkOpenGLCheckErrorMacro("failed after Render");
}