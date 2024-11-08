/**
 * @class   vtkF3DUIActor
 * @brief   An interface for UI context handler and renderer
 *
 * This interface is implemented by vtkF3DImguiActor if module is enabled
 */

#ifndef vtkF3DUIActor_h
#define vtkF3DUIActor_h

#include <vtkProp.h>

class vtkOpenGLRenderWindow;

class vtkF3DUIActor : public vtkProp
{
public:
  static vtkF3DUIActor* New();
  vtkTypeMacro(vtkF3DUIActor, vtkProp);

  virtual void Initialize(vtkOpenGLRenderWindow*) {}

  void SetFileNameVisibility(bool show);
  void ReportFileName(const std::string& filename);

  void SetFontFile(const std::string& font);

  int RenderOverlay(vtkViewport* vp) override;

  vtkF3DUIActor(const vtkF3DUIActor&) = delete;
  void operator=(const vtkF3DUIActor&) = delete;

protected:
  vtkF3DUIActor();
  ~vtkF3DUIActor() override;

  virtual void StartFrame(vtkOpenGLRenderWindow*) {}
  virtual void EndFrame(vtkOpenGLRenderWindow*) {}

  virtual void RenderFileName() {}

  bool FileNameVisible = false;
  std::string FileName = "";
  std::string FontFile = "";
};

#endif
