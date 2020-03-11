/**
 * @class   vtkF3DRenderer
 * @brief   The main viewer class
 *
 */

#ifndef vtkvtkF3DRenderer_h
#define vtkvtkF3DRenderer_h

#include "Config.h"
#include "F3DOptions.h"
#include "vtkF3DInteractorStyle.h"

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCornerAnnotation.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProgressBarWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>

struct F3DOptions;

class vtkF3DRenderer : public vtkOpenGLRenderer
{
public:
  static vtkF3DRenderer* New();
  vtkTypeMacro(vtkF3DRenderer, vtkOpenGLRenderer);

  void ShowAxis(bool show);
  void ShowGrid(bool show);
  void ShowScalarBar(bool show);
  void ShowEdge(bool show);
  void ShowTimer(bool show);
  void ShowFilename(bool show);
  void ShowScalars(bool show);
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetUseFXAAPass(bool use);
  void SetUsePointSprites(bool use);

  bool IsAxisVisible();
  bool IsGridVisible();
  bool IsScalarBarVisible();
  bool IsEdgeVisible();
  bool IsTimerVisible();
  bool IsFilenameVisible();
  bool AreScalarsVisible();
  bool UsingRaytracing();
  bool UsingRaytracingDenoiser();
  bool UsingDepthPeeling();
  bool UsingSSAOPass();
  bool UsingFXAAPass();
  bool UsingPointSprites();

  void Render() override;

  /**
   * Should be called after being added to a vtkRenderWindow
   */
  void Initialize(const F3DOptions& options, const std::string& fileInfo);

  vtkGetSmartPointerMacro(AxisWidget, vtkOrientationMarkerWidget);
  vtkSetSmartPointerMacro(AxisWidget, vtkOrientationMarkerWidget);

  //@{
  /**
   * Set/Get the scalar bar actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(ScalarBarActor, vtkActor2D);
  vtkSetSmartPointerMacro(ScalarBarActor, vtkActor2D);
  //@}

  //@{
  /**
   * Set/Get the geometry actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(GeometryActor, vtkActor);
  vtkSetSmartPointerMacro(GeometryActor, vtkActor);
  //@}

  //@{
  /**
   * Set/Get the polydata mapper actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PolyDataMapper, vtkMapper);
  vtkSetSmartPointerMacro(PolyDataMapper, vtkMapper);
  //@}

  //@{
  /**
   * Set/Get the point gaussian mapper actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PointGaussianMapper, vtkMapper);
  vtkSetSmartPointerMacro(PointGaussianMapper, vtkMapper);
  //@}

  void ShowOptions();

  void SetupRenderPasses();

protected:
  vtkF3DRenderer() = default;
  ~vtkF3DRenderer() override;

  F3DOptions Options;

  vtkNew<vtkActor> GridActor;
  vtkSmartPointer<vtkActor2D> ScalarBarActor;
  vtkSmartPointer<vtkActor> GeometryActor;
  vtkSmartPointer<vtkMapper> PolyDataMapper;
  vtkSmartPointer<vtkMapper> PointGaussianMapper;

  vtkSmartPointer<vtkOrientationMarkerWidget> AxisWidget;

  vtkNew<vtkCornerAnnotation> FilenameActor;

  // vtkCornerAnnotation building is too slow for the timer
  vtkNew<vtkTextActor> TimerActor;
  unsigned int Timer = 0;

  bool GridVisible;
  bool AxisVisible;
  bool EdgesVisible;
  bool TimerVisible;
  bool FilenameVisible;
  bool ScalarBarVisible;
  bool ScalarsVisible;
  bool UseRaytracing;
  bool UseRaytracingDenoiser;
  bool UseDepthPeeling;
  bool UseFXAAPass;
  bool UseSSAOPass;
  bool UsePointSprites;
};

#endif
