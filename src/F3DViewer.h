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
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>

#include <vtk_glew.h>

class F3DOptions;
class vtkImporter;
class vtkRenderWindow;
class vtkRenderer;

class F3DViewer
{
public:
  F3DViewer(F3DOptions *options, vtkImporter *importer);
  ~F3DViewer();

  int Start();

  void ShowAxis(bool show);
  void ShowGrid(bool show);
  void ShowScalarBar(bool show);
  void ShowEdge(bool show);
  void ShowTimer(bool show);

  bool IsAxisVisible();
  bool IsGridVisible();
  bool IsScalarBarVisible();
  bool IsEdgeVisible();
  bool IsTimerVisible();

  void Render();

  vtkRenderer* GetRenderer();

protected:
  void SetupWithOptions();

  void SetupRenderPasses();

  void SetProgress(double progress);

protected:
  F3DOptions *Options = nullptr;
  vtkImporter *Importer = nullptr;

  vtkNew<vtkRenderWindow> RenderWindow;
  vtkNew<vtkRenderer> Renderer;

  vtkNew<vtkActor> GridActor;
  vtkSmartPointer<vtkActor2D> ScalarBarActor;

  vtkNew<vtkProgressBarRepresentation> ProgressRepresentation;
  vtkNew<vtkProgressBarWidget> ProgressWidget;

  vtkNew<vtkOrientationMarkerWidget> AxisWidget;
  vtkNew<vtkRenderWindowInteractor> RenderWindowInteractor;
  vtkNew<vtkF3DInteractor> InteractorStyle;

  vtkNew<vtkTextActor> TimerActor;
  GLuint Timer;
};

#endif
