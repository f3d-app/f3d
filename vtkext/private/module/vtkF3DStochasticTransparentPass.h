/**
 * @class   vtkF3DStochasticTransparentPass
 * @brief   Implements a stochastic transparency rendering pass.
 *
 * This render pass implements a stochastic transparency technique based on
 * Enderton et al.'s "Stochastic Transparency". Instead of sorting and
 * compositing transparent objects, the pass applies a pseudo-random per-fragment
 * masking strategy so that over many independent samples the correct blended result is converged to
 * the expected result. This eliminates the need for draw-order sorting for translucent geometry and
 * can improve performance in complex scenes.
 *
 * Reference:
 * - Enderton, Eric, et al.
 *   "Stochastic transparency."
 *   Proceedings of the 2010 ACM SIGGRAPH symposium on Interactive 3D Graphics and Games. 2010.
 * - Kheradmand, Shakiba, et al.
 *   "StochasticSplats: Stochastic Rasterization for Sorting-Free 3D Gaussian Splatting."
 *   arXiv preprint arXiv:2503.24366 (2025).
 */

#ifndef vtkF3DStochasticTransparentPass_h
#define vtkF3DStochasticTransparentPass_h

#include "vtkOpenGLRenderPass.h"

class vtkInformationIntegerKey;
class vtkOpenGLRenderWindow;

class vtkF3DStochasticTransparentPass : public vtkOpenGLRenderPass
{
public:
  static vtkF3DStochasticTransparentPass* New();
  vtkTypeMacro(vtkF3DStochasticTransparentPass, vtkOpenGLRenderPass);

  void Render(const vtkRenderState* s) override;
  void ReleaseGraphicsResources(vtkWindow* w) override;

  void SetTranslucentPass(vtkRenderPass* translucentPass);
  void SetVolumetricPass(vtkRenderPass* volumetricPass);

  bool PreReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  bool SetShaderParameters(vtkShaderProgram* program, vtkAbstractMapper* mapper, vtkProp* prop,
    vtkOpenGLVertexArrayObject* VAO = nullptr) override;

  static vtkInformationIntegerKey* PropIndex();

private:
  vtkF3DStochasticTransparentPass();
  ~vtkF3DStochasticTransparentPass() override;

  vtkRenderPass* TranslucentPass = nullptr;
  vtkRenderPass* VolumetricPass = nullptr;
  vtkOpenGLRenderWindow* RenWin = nullptr;

  int Seed = 0;

  vtkF3DStochasticTransparentPass(const vtkF3DStochasticTransparentPass&) = delete;
  void operator=(const vtkF3DStochasticTransparentPass&) = delete;
};

#endif // vtkF3DStochasticTransparentPass_h
