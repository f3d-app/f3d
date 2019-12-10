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

  vtkSetMacro(FadeDistance, double);
  vtkGetMacro(FadeDistance, double);

  vtkSetMacro(UnitSquare, double);
  vtkGetMacro(UnitSquare, double);

  double* GetBounds() override;

protected:
  vtkF3DOpenGLGridMapper();
  ~vtkF3DOpenGLGridMapper() = default;

  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  void SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

  void BuildBufferObjects(vtkRenderer* ren, vtkActor* act) override;

  void RenderPiece(vtkRenderer* ren, vtkActor* actor) override;

  bool GetNeedToRebuildShaders(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* act) override;

  double FadeDistance = 10.0;
  double UnitSquare = 1.0;

private:
  vtkF3DOpenGLGridMapper(const vtkF3DOpenGLGridMapper&) = delete;
  void operator=(const vtkF3DOpenGLGridMapper&) = delete;
};

#endif
