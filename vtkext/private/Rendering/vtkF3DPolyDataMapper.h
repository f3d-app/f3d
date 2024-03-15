/**
 * @class   vtkF3DPolyDataMapper
 * @brief   Custom surface mapper used to include skinning and morphing for glTF format
 *
 */

#ifndef vtkF3DPolyDataMapper_h
#define vtkF3DPolyDataMapper_h

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkVersion.h>

class vtkF3DPolyDataMapper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkF3DPolyDataMapper* New();
  vtkTypeMacro(vtkF3DPolyDataMapper, vtkOpenGLPolyDataMapper);

  /**
   * Modify the shaders to include skinning and morphing capabilities
   */
  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  ///@{
  /**
   * Modify the shaders to use MatCap if enabled
   */
  void ReplaceShaderColor(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  void ReplaceShaderLight(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  void ReplaceShaderTCoord(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  ///@}

protected:
  vtkF3DPolyDataMapper();
  ~vtkF3DPolyDataMapper() override = default;
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20230902)
  /**
   * Call superclass then check for changes in the environment texture
   * in order to support correctly dynamic HDRIs.
   * Return true if the HDRI texture was changed since last call, false otherwise.
   * Integrated in VTK in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10456
   */
  bool GetNeedToRebuildShaders(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* act) override;
#endif

private:
  /**
   * Returns true if a MatCap texture is defined by the user and the actor has normals
   */
  bool RenderWithMatCap(vtkActor* actor);

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 3, 20230902)
  vtkMTimeType EnvTextureTime = 0;
  vtkTexture* EnvTexture = nullptr;
#endif

// SSBO support: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231108)
  vtkNew<vtkOpenGLBufferObject> JointMatrices;
#endif
};

#endif
