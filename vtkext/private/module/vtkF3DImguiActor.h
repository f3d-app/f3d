/**
 * @class   vtkF3DImguiActor
 * @brief   A ImGui context handler and renderer
 *
 */

#ifndef vtkF3DImguiActor_h
#define vtkF3DImguiActor_h

#include "vtkF3DUIActor.h"

#include <memory>

class vtkOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class vtkWindow;

class vtkF3DImguiActor : public vtkF3DUIActor
{
public:
  static vtkF3DImguiActor* New();
  vtkTypeMacro(vtkF3DImguiActor, vtkF3DUIActor);

  void ReleaseGraphicsResources(vtkWindow* w) override;

  vtkF3DImguiActor(const vtkF3DImguiActor&) = delete;
  void operator=(const vtkF3DImguiActor&) = delete;

protected:
  vtkF3DImguiActor();
  ~vtkF3DImguiActor() override;

private:
  struct Internals;
  std::unique_ptr<Internals> Pimpl;

  void Initialize(vtkOpenGLRenderWindow* renwin) override;

  void StartFrame(vtkOpenGLRenderWindow* renWin) override;
  void EndFrame(vtkOpenGLRenderWindow* renWin) override;

  void RenderFileName() override;
};

#endif
