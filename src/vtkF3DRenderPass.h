/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkF3DRenderPass.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkF3DRenderPass
 * @brief   Implement the basic render passes.
 *
 * vtkF3DRenderPass implements the render pass chain of F3D.
 * It consists of two main passes. The first one renders the background.
 * The second pass renders the dataset with different options (raytracing, SSAO, depth peeling, ...)
 * Once the two passes are rendered into textures, a final shader is applied to combine the
 * background (and optionally blur it using Bokeh depth of field) and the dataset image.
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DRenderPass_h
#define vtkF3DRenderPass_h

#include <vtkFramebufferPass.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

#include <memory>
#include <vector>

class vtkProp;

class vtkF3DRenderPass : public vtkRenderPass
{
public:
  static vtkF3DRenderPass* New();
  vtkTypeMacro(vtkF3DRenderPass, vtkRenderPass);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Render(const vtkRenderState* s) override;

  vtkGetMacro(UseRaytracing, bool);
  vtkSetMacro(UseRaytracing, bool);
  vtkBooleanMacro(UseRaytracing, bool);

  vtkGetMacro(UseSSAOPass, bool);
  vtkSetMacro(UseSSAOPass, bool);
  vtkBooleanMacro(UseSSAOPass, bool);

  vtkGetMacro(UseDepthPeelingPass, bool);
  vtkSetMacro(UseDepthPeelingPass, bool);
  vtkBooleanMacro(UseDepthPeelingPass, bool);

  vtkGetMacro(UseBlurBackground, bool);
  vtkSetMacro(UseBlurBackground, bool);
  vtkBooleanMacro(UseBlurBackground, bool);

  vtkGetVector6Macro(Bounds, double);
  vtkSetVector6Macro(Bounds, double);

protected:
  vtkF3DRenderPass() = default;
  ~vtkF3DRenderPass() override = default;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  void Initialize(const vtkRenderState* s);

  void Blend(const vtkRenderState* s);

  bool UseRaytracing = false;
  bool UseSSAOPass = false;
  bool UseDepthPeelingPass = false;
  bool UseBlurBackground = false;

  vtkSmartPointer<vtkFramebufferPass> BackgroundPass;
  vtkSmartPointer<vtkFramebufferPass> MainPass;

  double Bounds[6] = {};

  vtkMTimeType InitializeTime = 0;

  std::vector<vtkProp*> BackgroundProps;
  std::vector<vtkProp*> MainProps;

  std::shared_ptr<vtkOpenGLQuadHelper> BlendQuadHelper;

private:
  vtkF3DRenderPass(const vtkF3DRenderPass&) = delete;
  void operator=(const vtkF3DRenderPass&) = delete;
};

#endif
