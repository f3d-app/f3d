#include "vtkF3DBakeSSAOPass.h"

#include "vtkF3DBakeSSAOFS.h"
#include "vtkF3DRenderPassUtils.h"

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
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

#include <random>

vtkStandardNewMacro(vtkF3DBakeSSAOPass);

//------------------------------------------------------------------------------
vtkF3DBakeSSAOPass::vtkF3DBakeSSAOPass()
{
  this->SSAOTexture = vtkSmartPointer<vtkTextureObject>::New();
}

//------------------------------------------------------------------------------
vtkF3DBakeSSAOPass::~vtkF3DBakeSSAOPass() = default;

//------------------------------------------------------------------------------
void vtkF3DBakeSSAOPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  if (this->SSAOQuadHelper)
  {
    this->SSAOQuadHelper->ReleaseGraphicsResources(w);
    this->SSAOQuadHelper = nullptr;
  }
  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(w);
  }
  if (this->SSAOTexture)
  {
    this->SSAOTexture->ReleaseGraphicsResources(w);
  }
}

//------------------------------------------------------------------------------
void vtkF3DBakeSSAOPass::ComputeKernel()
{
  std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
  std::default_random_engine generator;

  this->Kernel.resize(3 * this->KernelSize);

  for (unsigned int i = 0; i < this->KernelSize; ++i)
  {
    float sample[3] = { randomFloats(generator) * 2.f - 1.f, randomFloats(generator) * 2.f - 1.f,
      randomFloats(generator) };

    // reject the sample if not in the hemisphere
    if (vtkMath::Norm(sample) > 1.0)
    {
      i--;
      continue;
    }

    // more samples closer to the point
    float scale = i / static_cast<float>(this->KernelSize);
    scale = 0.1f + 0.9f * scale * scale;
    vtkMath::MultiplyScalar(sample, scale);

    this->Kernel[3 * i] = sample[0];
    this->Kernel[3 * i + 1] = sample[1];
    this->Kernel[3 * i + 2] = sample[2];
  }
}

//------------------------------------------------------------------------------
void vtkF3DBakeSSAOPass::InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height)
{
  this->SSAOTexture->SetContext(renWin);
  if (!this->SSAOTexture->GetHandle())
  {
    this->SSAOTexture->SetFormat(GL_RED);
    this->SSAOTexture->SetInternalFormat(GL_R8);
    this->SSAOTexture->SetDataType(GL_UNSIGNED_BYTE);
    this->SSAOTexture->SetMinificationFilter(vtkTextureObject::Nearest);
    this->SSAOTexture->SetMagnificationFilter(vtkTextureObject::Nearest);
    this->SSAOTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->SSAOTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->SSAOTexture->Allocate2D(width, height, 1, VTK_UNSIGNED_CHAR);
  }
  this->SSAOTexture->Resize(width, height);

  if (!this->FrameBufferObject)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
    this->FrameBufferObject->Bind();
    this->FrameBufferObject->AddColorAttachment(0, this->SSAOTexture);
  }
  this->FrameBufferObject->SetContext(renWin);
}

//------------------------------------------------------------------------------
void vtkF3DBakeSSAOPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  if (!this->DepthTexture || !this->NormalTexture)
  {
    vtkWarningMacro("No depth or normal input texture.");
    return;
  }

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(r->GetRenderWindow());

  const int width = this->DepthTexture->GetWidth();
  const int height = this->DepthTexture->GetHeight();

  assert(this->NormalTexture->GetWidth() == width);
  assert(this->NormalTexture->GetHeight() == height);

  this->InitializeResources(renWin, width, height);

  if (this->SSAOQuadHelper && this->ShaderBuildTime < this->GetMTime())
  {
    this->SSAOQuadHelper = nullptr;
  }

  if (!this->SSAOQuadHelper)
  {
    this->ComputeKernel();

    std::string ssaoShader = vtkF3DBakeSSAOFS;
    vtkShaderProgram::Substitute(ssaoShader, "//VTK::KernelSize::Decl",
      "#define KERNEL_SIZE " + std::to_string(this->KernelSize));

    this->SSAOQuadHelper = std::make_shared<vtkOpenGLQuadHelper>(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), ssaoShader.c_str(), "");

    this->ShaderBuildTime.Modified();
  }

  vtkShaderProgram* ssaoProgram = this->SSAOQuadHelper->Program;
  if (!ssaoProgram || !ssaoProgram->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the SSAO shader program.");
    return;
  }

  auto projection =
    vtkSmartPointer<vtkMatrix4x4>::Take(r->GetActiveCamera()->GetProjectionTransformMatrix(r));
  vtkNew<vtkMatrix4x4> invProjection;
  vtkMatrix4x4::Invert(projection, invProjection);

  renWin->GetState()->PushFramebufferBindings();

  this->FrameBufferObject->StartNonOrtho(width, height);

  vtkF3DRenderPassUtils::SetRenderOptions(
    vtkOpenGLRenderer::SafeDownCast(r), { .Width = width, .Height = height });

  renWin->GetShaderCache()->ReadyShaderProgram(ssaoProgram);

  this->DepthTexture->Activate();
  this->NormalTexture->Activate();
  ssaoProgram->SetUniformi("texDepth", this->DepthTexture->GetTextureUnit());
  ssaoProgram->SetUniformi("texNormal", this->NormalTexture->GetTextureUnit());
  ssaoProgram->SetUniformi("texNoise", renWin->GetNoiseTextureUnit());
  ssaoProgram->SetUniform3fv("samples", this->KernelSize, this->Kernel.data());
  ssaoProgram->SetUniformf("kernelRadius", static_cast<float>(this->Radius));
  ssaoProgram->SetUniformf("kernelBias", static_cast<float>(this->Bias));
  ssaoProgram->SetUniformMatrix("matProjection", projection);
  ssaoProgram->SetUniformMatrix("matInvProjection", invProjection);

  this->SSAOQuadHelper->Render();

  this->NormalTexture->Deactivate();
  this->DepthTexture->Deactivate();

  renWin->GetState()->PopFramebufferBindings();

  vtkOpenGLCheckErrorMacro("failed after Render");
}
