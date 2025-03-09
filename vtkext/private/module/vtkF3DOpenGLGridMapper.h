/**
 * @class   vtkF3DOpenGLGridMapper
 * @brief   A mapper which display an infinite plane
 *
 */

#ifndef vtkF3DOpenGLGridMapper_h
#define vtkF3DOpenGLGridMapper_h

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkSmartPointer.h>

class vtkF3DOpenGLGridMapper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkF3DOpenGLGridMapper* New();
  vtkTypeMacro(vtkF3DOpenGLGridMapper, vtkOpenGLPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the origin of the axes relative to the actor position
   */
  vtkSetVector3Macro(OriginOffset, double);

  /**
   * Set the distance where the grid disappear.
   */
  vtkSetMacro(FadeDistance, double);

  /**
   * Set the size of a square on the grid.
   */
  vtkSetMacro(UnitSquare, double);

  /**
   * Set the number of minor lines per square on the grid.
   */
  vtkSetMacro(Subdivisions, int);

  /**
   * Set the color (RGBA) of the first axis
   */
  vtkSetVector4Macro(Axis1Color, float);

  /**
   * Set the color (RGBA) of the second axis
   */
  vtkSetVector4Macro(Axis2Color, float);

  using vtkOpenGLPolyDataMapper::GetBounds;
  double* GetBounds() override;

  vtkF3DOpenGLGridMapper(const vtkF3DOpenGLGridMapper&) = delete;
  void operator=(const vtkF3DOpenGLGridMapper&) = delete;

protected:
  vtkF3DOpenGLGridMapper();
  ~vtkF3DOpenGLGridMapper() override = default;

  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  void SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

  void BuildBufferObjects(vtkRenderer* ren, vtkActor* act) override;

  void RenderPiece(vtkRenderer* ren, vtkActor* actor) override;

  bool GetNeedToRebuildShaders(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* act) override;

  double OriginOffset[3] = { 0.0, 0.0, 0.0 };
  double FadeDistance = 10.0;
  double UnitSquare = 1.0;
  int Subdivisions = 10;
  float Axis1Color[4] = { 0.0, 0.0, 0.0, 1.0 };
  float Axis2Color[4] = { 0.0, 0.0, 0.0, 1.0 };
};

#endif
