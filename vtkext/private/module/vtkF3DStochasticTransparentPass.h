// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @class   vtkF3DStochasticTransparentPass
 * @brief   Implements the stochastic transparent algorithm.
 *
 * TODO: link to paper
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
