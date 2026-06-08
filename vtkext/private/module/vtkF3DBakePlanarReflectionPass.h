/**
 * @class   vtkF3DBakePlanarReflectionPass
 * @brief   Implements a planar reflection baking rendering pass.
 *
 * This render pass is used to bake the planar reflection of the scene into a texture
 * that can be used by the grid. It is used in the BakeReflectionPass of vtkF3DRenderPass.
 * The reflection is baked by rendering the scene with a reflected camera based
 * on the reflection actor orientation and position which is expected to be a plane.
 */

#ifndef vtkF3DBakePlanarReflectionPass_h
#define vtkF3DBakePlanarReflectionPass_h

#include "vtkActor.h"
#include "vtkOpenGLRenderPass.h"
#include "vtkSmartPointer.h"

class vtkF3DBakePlanarReflectionPass : public vtkOpenGLRenderPass
{
public:
  static vtkF3DBakePlanarReflectionPass* New();
  vtkTypeMacro(vtkF3DBakePlanarReflectionPass, vtkOpenGLRenderPass);

  /**
   * Render the pass
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Set the actor used to bake the reflection. Usually the grid actor.
   */
  vtkSetSmartPointerMacro(ReflectionActor, vtkActor);

private:
  vtkF3DBakePlanarReflectionPass();
  ~vtkF3DBakePlanarReflectionPass() override;

  vtkF3DBakePlanarReflectionPass(const vtkF3DBakePlanarReflectionPass&) = delete;
  void operator=(const vtkF3DBakePlanarReflectionPass&) = delete;

  vtkSmartPointer<vtkActor> ReflectionActor;
};

#endif // vtkF3DBakePlanarReflectionPass_h
