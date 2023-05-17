#ifndef vtkF3DDropZoneActor_h
#define vtkF3DDropZoneActor_h

#include <vtkActor2D.h>

#include <vtkNew.h>

class vtkTextMapper;
class vtkTextProperty;
class vtkPolyData;
class vtkPolyDataMapper2D;

class vtkF3DDropZoneActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkF3DDropZoneActor, vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkF3DDropZoneActor* New();

  vtkTextProperty* GetTextProperty();

  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;

protected:
  vtkF3DDropZoneActor();
  ~vtkF3DDropZoneActor() override;

private:
  vtkF3DDropZoneActor(const vtkF3DDropZoneActor&) = delete;
  void operator=(const vtkF3DDropZoneActor&) = delete;

  void BuildBorderGeometry(vtkViewport* viewport);

  vtkNew<vtkActor2D> TextActor;
  vtkNew<vtkTextMapper> TextMapper;

  int ComputedBorderViewportSize[2] = {-1, -1};
  vtkNew<vtkPolyData> BorderPolyData;
  vtkNew<vtkActor2D> BorderActor;
  vtkNew<vtkPolyDataMapper2D> BorderMapper;
};

#endif
