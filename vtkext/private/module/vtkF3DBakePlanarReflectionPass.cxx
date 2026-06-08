#include "vtkF3DBakePlanarReflectionPass.h"

#include <vtkCamera.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkglad/include/glad/gl.h>

vtkStandardNewMacro(vtkF3DBakePlanarReflectionPass);

//------------------------------------------------------------------------------
vtkF3DBakePlanarReflectionPass::vtkF3DBakePlanarReflectionPass() = default;

//------------------------------------------------------------------------------
vtkF3DBakePlanarReflectionPass::~vtkF3DBakePlanarReflectionPass() = default;

//------------------------------------------------------------------------------
void vtkF3DBakePlanarReflectionPass::Render(const vtkRenderState* s)
{
  assert(this->ReflectionActor != nullptr);

  vtkNew<vtkMatrix4x4> actorMatrix;
  this->ReflectionActor->GetMatrix(actorMatrix);

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

  // save the camera for later restore
  vtkCamera* originalCam = s->GetRenderer()->GetActiveCamera();

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

  vtkNew<vtkCamera> reflectedCam;
  reflectedCam->DeepCopy(originalCam);
  reflectedCam->SetPosition(pos[0], pos[1], pos[2]);
  reflectedCam->SetFocalPoint(foc[0], foc[1], foc[2]);
  reflectedCam->SetViewUp(up[0], up[1], up[2]);

  s->GetRenderer()->SetActiveCamera(reflectedCam);

  this->PreRender(s);

  this->NumberOfRenderedProps = 0;

  int numProps = s->GetPropArrayCount();
  for (int i = 0; i < numProps; ++i)
  {
    this->NumberOfRenderedProps += s->GetPropArray()[i]->RenderOpaqueGeometry(s->GetRenderer());
  }

  for (int i = 0; i < numProps; ++i)
  {
    this->NumberOfRenderedProps +=
      s->GetPropArray()[i]->RenderTranslucentPolygonalGeometry(s->GetRenderer());
  }

  this->PostRender(s);

  // restore original camera
  s->GetRenderer()->SetActiveCamera(originalCam);
}
