#include "vtkF3DExternalRenderPass.h"

#include "vtkF3DRenderPassUtils.h"

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

#include <string>

#if F3D_MODULE_ANARI
#include <vtkAnariDevice.h>
#include <vtkAnariPass.h>
#endif

#if F3D_MODULE_RAYTRACING
#include <vtkOSPRayPass.h>
#endif

vtkStandardNewMacro(vtkF3DExternalRenderPass);

//------------------------------------------------------------------------------
vtkF3DExternalRenderPass::vtkF3DExternalRenderPass()
{
  this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
}

//------------------------------------------------------------------------------
vtkF3DExternalRenderPass::~vtkF3DExternalRenderPass() = default;

//------------------------------------------------------------------------------
bool vtkF3DExternalRenderPass::IsBackendAvailable(Backend backend)
{
  switch (backend)
  {
    case Backend::OSPRAY:
#if F3D_MODULE_RAYTRACING
      return true;
#else
      return false;
#endif
    case Backend::ANARI:
#if F3D_MODULE_ANARI
      return true;
#else
      return false;
#endif
  }
  return false;
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderPass::SetBackend(Backend backend)
{
  if (this->CurrentBackend != backend)
  {
    this->CurrentBackend = backend;
    this->DelegatePass = nullptr;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderPass::SetAnariLibrary(const std::string& library)
{
  if (this->AnariLibrary != library)
  {
    this->AnariLibrary = library;
    if (this->CurrentBackend == Backend::ANARI)
    {
      this->DelegatePass = nullptr;
    }
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderPass::CreateDelegatePass()
{
  switch (this->CurrentBackend)
  {
    case Backend::OSPRAY:
#if F3D_MODULE_RAYTRACING
      this->DelegatePass = vtkSmartPointer<vtkOSPRayPass>::New();
#endif
      break;
    case Backend::ANARI:
#if F3D_MODULE_ANARI
    {
      auto anariP = vtkSmartPointer<vtkAnariPass>::New();
      anariP->GetAnariDevice()->SetupAnariDeviceFromLibrary(
        this->AnariLibrary.c_str(), "default", false);
      this->DelegatePass = anariP;
    }
#endif
    break;
  }
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  if (this->DelegatePass)
  {
    this->DelegatePass->ReleaseGraphicsResources(w);
  }

  if (this->QuadHelper)
  {
    this->QuadHelper->ReleaseGraphicsResources(w);
    this->QuadHelper = nullptr;
  }
  this->FrameBufferObject = nullptr;
  this->ColorTexture->ReleaseGraphicsResources(w);
}

//------------------------------------------------------------------------------
void vtkF3DExternalRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  if (!this->DelegatePass)
  {
    this->CreateDelegatePass();
  }

  assert(this->DelegatePass);

  vtkRenderer* renderer = s->GetRenderer();
  vtkOpenGLRenderer* glRenderer = vtkOpenGLRenderer::SafeDownCast(renderer);
  vtkOpenGLRenderWindow* renWin =
    vtkOpenGLRenderWindow::SafeDownCast(glRenderer->GetRenderWindow());

  int width = 0;
  int height = 0;
  vtkF3DRenderPassUtils::ComputeRenderSize(s, this->ResolutionScale, width, height);

  renWin->GetState()->PushFramebufferBindings();

  this->ColorTexture->SetContext(renWin);
  if (!this->ColorTexture->GetHandle())
  {
    this->ColorTexture->SetInternalFormat(GL_RGBA16F);
    this->ColorTexture->SetDataType(GL_FLOAT);
    this->ColorTexture->Create2D(width, height, 4, VTK_FLOAT, false);
  }
  this->ColorTexture->Resize(width, height);

  if (!this->FrameBufferObject)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
    this->FrameBufferObject->Bind();
    this->FrameBufferObject->AddColorAttachment(0, this->ColorTexture);
    this->FrameBufferObject->AddDepthAttachment();
  }
  this->FrameBufferObject->SetContext(renWin);

  this->FrameBufferObject->StartNonOrtho(width, height);

  vtkF3DRenderPassUtils::SetRenderOptions(glRenderer, { .Width = width, .Height = height });

  // Workaround to have a black background for the external renderer
  // OSPRay does a renderer clear internally
  double bgColor[3];
  renderer->GetBackground(bgColor);
  renderer->SetBackground(0.0, 0.0, 0.0);

  vtkRenderState delegateState(s->GetRenderer());
  delegateState.SetPropArrayAndCount(s->GetPropArray(), s->GetPropArrayCount());
  delegateState.SetFrameBuffer(this->FrameBufferObject);
  this->DelegatePass->Render(&delegateState);
  this->NumberOfRenderedProps += this->DelegatePass->GetNumberOfRenderedProps();

  // restore background
  renderer->SetBackground(bgColor);

  renWin->GetState()->PopFramebufferBindings();

  vtkF3DRenderPassUtils::SetRenderOptions(glRenderer,
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestNone,
      .BlendMode = vtkF3DRenderPassUtils::BlendPremultiplied,
      .Width = width,
      .Height = height });

  if (this->QuadHelper && this->QuadHelper->ShaderChangeValue < this->GetMTime())
  {
    this->QuadHelper = nullptr;
  }

  if (!this->QuadHelper)
  {
    std::string fsSource = vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();
    vtkShaderProgram::Substitute(fsSource, "//VTK::FSQ::Decl", "uniform sampler2D source;\n");

    std::string impl = "  vec4 color = texture(source, texCoord);\n";
    if (!this->OutputColorIsLinear)
    {
      // convert to straight alpha, then to sRGB, then back to premultiplied alpha
      impl += "  if (color.a > 0.0)\n"
              "    color.rgb = pow(color.rgb / color.a, vec3(1.0 / 2.2)) * color.a;\n";
    }
    impl += "  gl_FragData[0] = color;\n";
    vtkShaderProgram::Substitute(fsSource, "//VTK::FSQ::Impl", impl);

    this->QuadHelper = std::make_shared<vtkOpenGLQuadHelper>(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), fsSource.c_str(), "");
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  this->ColorTexture->Activate();
  this->QuadHelper->Program->SetUniformi("source", this->ColorTexture->GetTextureUnit());
  this->QuadHelper->Render();
  this->ColorTexture->Deactivate();

  vtkOpenGLCheckErrorMacro("failed after Render");
}
