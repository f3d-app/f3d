#include "vtkF3DDepthPrePass.h"

#include "vtkF3DRenderPassUtils.h"

#include <vtkActor.h>
#include <vtkCameraPass.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
#include <vtkPointGaussianMapper.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DDepthPrePass);

//------------------------------------------------------------------------------
vtkF3DDepthPrePass::vtkF3DDepthPrePass()
{
  this->NormalTexture = vtkSmartPointer<vtkTextureObject>::New();
  this->DepthTexture = vtkSmartPointer<vtkTextureObject>::New();
  this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
}

//------------------------------------------------------------------------------
vtkF3DDepthPrePass::~vtkF3DDepthPrePass() = default;

//------------------------------------------------------------------------------
void vtkF3DDepthPrePass::SetDelegatePass(vtkRenderPass* delegate)
{
  if (this->DelegatePass != delegate)
  {
    this->DelegatePass = delegate;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DDepthPrePass::InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height)
{
  this->NormalTexture->SetContext(renWin);
  if (!this->NormalTexture->GetHandle())
  {
    this->NormalTexture->SetFormat(GL_RG);
    // TODO: is RG8 sufficient for normals encoding? (RG16 not available on GLES)
    this->NormalTexture->SetInternalFormat(GL_RG16);
    this->NormalTexture->SetDataType(GL_UNSIGNED_SHORT);
    this->NormalTexture->SetMinificationFilter(vtkTextureObject::Nearest);
    this->NormalTexture->SetMagnificationFilter(vtkTextureObject::Nearest);
    this->NormalTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->NormalTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->NormalTexture->Allocate2D(width, height, 2, VTK_UNSIGNED_SHORT);
  }
  this->NormalTexture->Resize(width, height);

  this->DepthTexture->SetContext(renWin);
  if (!this->DepthTexture->GetHandle())
  {
    this->DepthTexture->AllocateDepth(width, height, vtkTextureObject::Float32);
    this->DepthTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->DepthTexture->SetWrapT(vtkTextureObject::ClampToEdge);
  }
  this->DepthTexture->Resize(width, height);

  this->FrameBufferObject->SetContext(renWin);
  this->FrameBufferObject->Bind();
  this->FrameBufferObject->AddColorAttachment(0, this->NormalTexture);
  this->FrameBufferObject->AddDepthAttachment(this->DepthTexture);
}

//------------------------------------------------------------------------------
void vtkF3DDepthPrePass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  if (!this->DelegatePass)
  {
    vtkWarningMacro(<< " no delegate.");
    return;
  }

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  int width = 0;
  int height = 0;
  vtkF3DRenderPassUtils::ComputeRenderSize(s, this->ResolutionScale, width, height);

  ostate->PushFramebufferBindings();

  this->InitializeResources(renWin, width, height);

  this->FrameBufferObject->StartNonOrtho(width, height);

  vtkF3DRenderPassUtils::SetRenderOptions(vtkOpenGLRenderer::SafeDownCast(r),
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestLEqual,
      .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .ClearColor = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f },
      .ClearDepth = 1.f,
      .WriteDepth = true,
      .Width = width,
      .Height = height });

  this->PreRender(s);
  this->DelegatePass->Render(s);
  this->PostRender(s);

  this->NumberOfRenderedProps = this->DelegatePass->GetNumberOfRenderedProps();

  ostate->PopFramebufferBindings();

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
bool vtkF3DDepthPrePass::PostReplaceShaderValues(
  std::string&, std::string&, std::string& fragmentShader, vtkAbstractMapper*, vtkProp*)
{
  constexpr std::string_view OCTAHEDRAL_ENCODE =
    "vec2 octahedralEncode(vec3 n)\n"
    "{\n"
    "  n /= (abs(n.x) + abs(n.y) + abs(n.z));\n"
    "  vec2 e = n.xy;\n"
    "  if (n.z < 0.0)\n"
    "  {\n"
    "    e = (1.0 - abs(e.yx)) * vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);\n"
    "  }\n"
    "  return e * 0.5 + 0.5;\n"
    "}\n";

  vtkShaderProgram::Substitute(fragmentShader, "//VTK::System::Dec",
    std::string("//VTK::System::Dec\n") + OCTAHEDRAL_ENCODE.data(), false);

  // check if normal is available in the fragment shader
  // TODO: can we do something smarter to avoid switching shaders?
  const bool hasNormal = fragmentShader.find("vec3 normalVCVSOutput") != std::string::npos;
  const std::string normal = hasNormal ? "normalize(normalVCVSOutput)" : "vec3(0.0, 0.0, 1.0)";

  vtkShaderProgram::Substitute(fragmentShader, "//VTK::Picking::Impl",
    "  gl_FragData[0] = vec4(octahedralEncode(" + normal + "), 0.0, 1.0);\n//VTK::Picking::Impl",
    false);

  return true;
}

//------------------------------------------------------------------------------
void vtkF3DDepthPrePass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(w);
  }
  if (this->NormalTexture)
  {
    this->NormalTexture->ReleaseGraphicsResources(w);
  }
  if (this->DepthTexture)
  {
    this->DepthTexture->ReleaseGraphicsResources(w);
  }
}
