/**
 * @class   vtkF3DDropZoneActor
 * @brief   A F3D dedicated drop zone actor
 *
 * This actor show a drop zone with a fixed text
 * in the center of the screen. The main feature
 * is the geometry of the dropzone that adapts to the size of
 * of the viewport.
 */

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
  static vtkF3DDropZoneActor* New();
  vtkTypeMacro(vtkF3DDropZoneActor, vtkActor2D);

  /**
   * Set the drop text to display
   */
  vtkSetMacro(DropText, std::string);

  /**
   * Get the text property used to display text in this actor
   */
  vtkTextProperty* GetTextProperty();

  /**
   * Release private actor graphic resources
   */
  void ReleaseGraphicsResources(vtkWindow*) override;

  /**
   * Render private actors overlay
   */
  int RenderOverlay(vtkViewport* viewport) override;

  /**
   * Reimplemented for noop
   */
  int RenderTranslucentPolygonalGeometry(vtkViewport* vtkNotUsed(viewport)) override
  {
    return 0;
  }

  /**
   * Reimplemented for noop
   */
  int RenderOpaqueGeometry(vtkViewport* vtkNotUsed(viewport)) override
  {
    return 0;
  }

protected:
  vtkF3DDropZoneActor();
  ~vtkF3DDropZoneActor() override;

private:
  vtkF3DDropZoneActor(const vtkF3DDropZoneActor&) = delete;
  void operator=(const vtkF3DDropZoneActor&) = delete;

  /**
   * Build the border geometry if the size of the viewport changed
   * Return true on success, false otherwise
   */
  bool BuildBorderGeometry(vtkViewport* viewport);

  std::string DropText;
  vtkNew<vtkActor2D> TextActor;
  vtkNew<vtkTextMapper> TextMapper;
  int ComputedBorderViewportSize[2] = { -1, -1 };
  vtkNew<vtkPolyData> BorderPolyData;
  vtkNew<vtkActor2D> BorderActor;
  vtkNew<vtkPolyDataMapper2D> BorderMapper;
};

#endif
