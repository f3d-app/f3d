/**
 * @class   vtkF3DPolyDataMapper
 * @brief   Custom surface mapper used to include F3D features
 *
 * This mapper is used to add support for SSBO skinning and
 * backward compatibility with old VTK versions for unlit materials.
 */

#ifndef vtkF3DPolyDataMapper_h
#define vtkF3DPolyDataMapper_h

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkVersion.h>
#include <vtkWeakPointer.h>

class vtkFloatArray;
class vtkOpenGLRenderWindow;

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
   * Modify the shaders to handle color space
   */
  void ReplaceShaderColor(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  void ReplaceShaderLight(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  ///@}

  /**
   * Set the SSBO for skinning if needed
   */
  void SetCustomUniforms(vtkOpenGLHelper& cellBO, vtkActor* actor) override;

  void ReleaseGraphicsResources(vtkWindow* window) override;

  // Camera framing must use the deformed surface even when CPU points stay in bind pose.
  double* GetBounds() override;
  using vtkOpenGLPolyDataMapper::GetBounds;

  bool PrepareSparseMorphing(vtkOpenGLRenderWindow* window, vtkPolyData* polydata);
  bool GetHasSparseMorphing() const
  {
    return this->HasSparseMorphing;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20260730)
  bool FragmentShaderUsesPrimitiveID(vtkRenderer*, vtkActor*) override;
#endif

  /**
   * Changes flags dictating the decode scalar colors for raytracing,
   * which expects linear RGB.
   * OpenGL performs this conversion in its PBR shader instead.
   */
  void SetUseLinearColorSpace(bool use);

  /**
   * Our vtkPolyDataMapper::MapScalar() override which computes linear RGB from sRGB
   * if needed.
   */
  vtkUnsignedCharArray* MapScalars(vtkDataSet* input, double alpha, int& cellFlag) override;

protected:
  vtkF3DPolyDataMapper() = default;
  ~vtkF3DPolyDataMapper() override = default;

  // Requires a current OpenGL context supporting shader storage buffers.
  bool UpdateMorphingBuffers(vtkPolyData* polydata);
  vtkNew<vtkOpenGLBufferObject> BlendShapeOffsets;
  vtkNew<vtkOpenGLBufferObject> BlendShapeWeights;

private:
  bool HasSparseMorphing = false;
  vtkWeakPointer<vtkPolyData> BoundsInput;
  vtkTimeStamp MorphingBoundsTime;
  double MorphingBounds[6];
  vtkWeakPointer<vtkFloatArray> UploadedBlendShapeOffsets;
  vtkWeakPointer<vtkFloatArray> UploadedBlendShapeWeights;
  vtkTimeStamp BlendShapeOffsetsUploadTime;
  vtkTimeStamp BlendShapeWeightsUploadTime;
  vtkNew<vtkOpenGLBufferObject> JointMatrices;
  bool HasSSBOSkinning = false;

  // Cached linear color scalars used in vtkMapper::ColorTextureMap.
  bool UseLinearColorSpace = false;
  vtkSmartPointer<vtkUnsignedCharArray> LinearColorTextureScalars;
};

#endif
