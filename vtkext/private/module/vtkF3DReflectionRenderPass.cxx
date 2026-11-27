#include "vtkF3DReflectionRenderPass.h"

#include "vtkF3DOpenGLGridMapper.h"
#include "vtkF3DRenderPassUtils.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DReflectionRenderPass);

//------------------------------------------------------------------------------
vtkF3DReflectionRenderPass::vtkF3DReflectionRenderPass()
  : ColorTexture(vtkSmartPointer<vtkTextureObject>::New())
  , DepthTexture(vtkSmartPointer<vtkTextureObject>::New())
{
}

//------------------------------------------------------------------------------
vtkF3DReflectionRenderPass::~vtkF3DReflectionRenderPass() = default;

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::SetDelegatePass(vtkRenderPass* delegate)
{
  if (this->DelegatePass != delegate)
  {
    this->DelegatePass = delegate;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::SetReflectionMatrix(vtkMatrix4x4* matrix)
{
  if (this->ReflectionMatrix != matrix)
  {
    this->ReflectionMatrix = matrix;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  this->FrameBufferObject = nullptr;
  this->ColorTexture->ReleaseGraphicsResources(w);
  this->DepthTexture->ReleaseGraphicsResources(w);
}

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::InitializeResources(
  vtkOpenGLRenderWindow* renWin, int width, int height)
{
  this->ColorTexture->SetContext(renWin);
  if (!this->ColorTexture->GetHandle())
  {
    this->ColorTexture->SetInternalFormat(GL_RGBA16F);
    this->ColorTexture->SetDataType(GL_FLOAT);
    this->ColorTexture->Create2D(width, height, 4, VTK_UNSIGNED_CHAR, false);
  }
  this->ColorTexture->Resize(width, height);

  this->DepthTexture->SetContext(renWin);
  if (!this->DepthTexture->GetHandle())
  {
    this->DepthTexture->AllocateDepth(width, height, vtkTextureObject::Float32);
  }
  this->DepthTexture->Resize(width, height);

  if (!this->FrameBufferObject)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
  }
  this->FrameBufferObject->SetContext(renWin);
  this->FrameBufferObject->Bind();
  this->FrameBufferObject->AddColorAttachment(0, this->ColorTexture);
  this->FrameBufferObject->AddDepthAttachment(this->DepthTexture);
}

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  assert(this->DelegatePass);
  assert(this->ReflectionMatrix);

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  int width = 0;
  int height = 0;
  vtkF3DRenderPassUtils::ComputeRenderSize(s, this->ResolutionScale, width, height);

  this->InitializeResources(renWin, width, height);

  // the grid must not be reflected
  this->ReflectedProps.clear();
  vtkProp* const* props = s->GetPropArray();
  for (int i = 0; i < s->GetPropArrayCount(); i++)
  {
    vtkActor* actor = vtkActor::SafeDownCast(props[i]);
    if (!actor || vtkF3DOpenGLGridMapper::SafeDownCast(actor->GetMapper()) == nullptr)
    {
      this->ReflectedProps.push_back(props[i]);
    }
  }

  vtkOpenGLFramebufferObject* fbo = this->FrameBufferObject;

  vtkRenderState s2(r);
  s2.SetPropArrayAndCount(
    this->ReflectedProps.data(), static_cast<int>(this->ReflectedProps.size()));
  s2.SetFrameBuffer(fbo);

  ostate->PushFramebufferBindings();

  fbo->StartNonOrtho(width, height);

  const vtkSmartPointer<vtkCamera> originalCam = r->GetActiveCamera();
  const vtkNew<vtkCamera> reflectedCam;
  this->ReflectCamera(originalCam, reflectedCam);
  r->SetActiveCamera(reflectedCam);

  vtkF3DRenderPassUtils::SetRenderOptions(vtkOpenGLRenderer::SafeDownCast(r),
    { .DepthTestMode = vtkF3DRenderPassUtils::DepthTestLEqual,
      .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .ClearColor = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f },
      .ClearDepth = 1.f,
      .WriteDepth = true,
      .Width = width,
      .Height = height });

  this->DelegatePass->Render(&s2);
  this->NumberOfRenderedProps = this->DelegatePass->GetNumberOfRenderedProps();

  // restore camera and mark it dirty to ensure headlights are recomputed
  originalCam->Modified();
  r->SetActiveCamera(originalCam);

  ostate->PopFramebufferBindings();

  vtkOpenGLCheckErrorMacro("failed after Render");
}

//------------------------------------------------------------------------------
void vtkF3DReflectionRenderPass::ReflectCamera(vtkCamera* originalCam, vtkCamera* reflectedCam)
{
  vtkMatrix4x4* actorMatrix = this->ReflectionMatrix;

  // compute the reflection matrix
  // see https://www.opengl.org/archives/resources/code/samples/sig99/advanced99/notes/node159.html
  double n[3] = { actorMatrix->GetElement(0, 1), actorMatrix->GetElement(1, 1),
    actorMatrix->GetElement(2, 1) };
  vtkMath::Normalize(n);

  // Plane center point
  const double p[3] = { actorMatrix->GetElement(0, 3), actorMatrix->GetElement(1, 3),
    actorMatrix->GetElement(2, 3) };

  const double a = n[0], b = n[1], c = n[2];
  const double d = a * p[0] + b * p[1] + c * p[2];

  vtkNew<vtkMatrix4x4> reflectionMatrix;
  reflectionMatrix->SetElement(0, 0, 1 - 2 * a * a);
  reflectionMatrix->SetElement(0, 1, -2 * a * b);
  reflectionMatrix->SetElement(0, 2, -2 * a * c);
  reflectionMatrix->SetElement(0, 3, 2 * a * d);

  reflectionMatrix->SetElement(1, 0, -2 * a * b);
  reflectionMatrix->SetElement(1, 1, 1 - 2 * b * b);
  reflectionMatrix->SetElement(1, 2, -2 * b * c);
  reflectionMatrix->SetElement(1, 3, 2 * b * d);

  reflectionMatrix->SetElement(2, 0, -2 * a * c);
  reflectionMatrix->SetElement(2, 1, -2 * b * c);
  reflectionMatrix->SetElement(2, 2, 1 - 2 * c * c);
  reflectionMatrix->SetElement(2, 3, 2 * c * d);

  reflectionMatrix->SetElement(3, 0, 0);
  reflectionMatrix->SetElement(3, 1, 0);
  reflectionMatrix->SetElement(3, 2, 0);
  reflectionMatrix->SetElement(3, 3, 1);

  // Reflect position
  double pos[4] = { originalCam->GetPosition()[0], originalCam->GetPosition()[1],
    originalCam->GetPosition()[2], 1.0 };
  reflectionMatrix->MultiplyPoint(pos, pos);

  // Reflect focal point
  double foc[4] = { originalCam->GetFocalPoint()[0], originalCam->GetFocalPoint()[1],
    originalCam->GetFocalPoint()[2], 1.0 };
  reflectionMatrix->MultiplyPoint(foc, foc);

  // Reflect up vector (direction, w=0)
  double up[4] = { originalCam->GetViewUp()[0], originalCam->GetViewUp()[1],
    originalCam->GetViewUp()[2], 0.0 };
  reflectionMatrix->MultiplyPoint(up, up);

  reflectedCam->DeepCopy(originalCam);
  reflectedCam->SetPosition(pos[0], pos[1], pos[2]);
  reflectedCam->SetFocalPoint(foc[0], foc[1], foc[2]);
  reflectedCam->SetViewUp(up[0], up[1], up[2]);
}
