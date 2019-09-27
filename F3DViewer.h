/**
 * @class   F3DViewer
 * @brief   The main viewer class
 *
 */

#ifndef vtkF3DViewer_h
#define vtkF3DViewer_h

#include "Config.h"
#include "vtkF3DInteractor.h"

#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderWindowInteractor.h>

class vtkF3DGenericImporter;
class F3DOptions;
class vtkRenderer;
class vtkRenderWindow;

class F3DViewer
{
public:
  F3DViewer(F3DOptions *options, vtkF3DGenericImporter *importer);
  ~F3DViewer() = default;

  void Start();

  void ShowAxis(bool show);
  void ShowGrid(bool show);

  bool IsAxisVisible();
  bool IsGridVisible();

  void Render();

protected:
  void SetupWithOptions();

  void SetupRenderPasses();

protected:
  F3DOptions *Options = nullptr;
  vtkF3DGenericImporter *Importer = nullptr;

  vtkRenderWindow *RenderWindow = nullptr;
  vtkRenderer *Renderer = nullptr;

  vtkNew<vtkActor> GridActor;

  vtkNew<vtkOrientationMarkerWidget> AxisWidget;
  vtkNew<vtkRenderWindowInteractor> RenderWindowInteractor;
  vtkNew<vtkF3DInteractor> InteractorStyle;
};

#endif
