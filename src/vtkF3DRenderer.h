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
#include <vtkSkybox.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
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
  void ShowMetaData(bool show);
  void ShowFilename(bool show);
  void ShowCheatSheet(bool show);
  void ShowScalars(bool show);
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetUseFXAAPass(bool use);
  void SetUsePointSprites(bool use);
  void SetUseToneMappingPass(bool use);
  void SetUseVolume(bool use);
  void SetUseInverseOpacityFunction(bool use);
  void SetUseBlurBackground(bool use);
  void SetUseTrackball(bool use);

  bool IsAxisVisible();
  bool IsGridVisible();
  bool IsScalarBarVisible();
  bool IsEdgeVisible();
  bool IsTimerVisible();
  bool IsFilenameVisible();
  bool IsMetaDataVisible();
  bool IsCheatSheetVisible();
  bool AreScalarsVisible();
  bool UsingRaytracing();
  bool UsingRaytracingDenoiser();
  bool UsingDepthPeelingPass();
  bool UsingSSAOPass();
  bool UsingFXAAPass();
  bool UsingPointSprites();
  bool UsingToneMappingPass();
  bool UsingVolume();
  bool UsingInverseOpacityFunction();
  bool UsingBlurBackground();
  bool UsingTrackball();

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
   * Set/Get the point sprites actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PointSpritesActor, vtkActor);
  vtkSetSmartPointerMacro(PointSpritesActor, vtkActor);
  //@}

  //@{
  /**
   * Set/Get the volume prop, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(VolumeProp, vtkVolume);
  vtkSetSmartPointerMacro(VolumeProp, vtkVolume);
  //@}

  //@{
  /**
   * Set/Get the polydata mapper, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PolyDataMapper, vtkMapper);
  vtkSetSmartPointerMacro(PolyDataMapper, vtkMapper);
  //@}

  //@{
  /**
   * Set/Get the point gaussian mapper, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PointGaussianMapper, vtkMapper);
  vtkSetSmartPointerMacro(PointGaussianMapper, vtkMapper);
  //@}

  //@{
  /**
   * Set/Get the volume mapper, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  vtkSetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  //@}

  /**
   * Set the ScalarsAvailable flag to inform the renderer that
   * scalars rendering can be done.
   */
  vtkSetMacro(ScalarsAvailable, bool);

  /**
   * Get the OpenGL skybox
   */
  vtkGetObjectMacro(Skybox, vtkSkybox);

  void ShowOptions();

  void SetupRenderPasses();

  /**
   * Initialize the camera position, focal point,
   * view up and view angle according to the options if any
   */
  void InitializeCamera();

  /**
   * Reset the camera to its initial parameters
   */
  void ResetCamera() override;

  /**
   * Dump useful scene state. Currently supports only camera state.
   */
  void DumpSceneState();

  /**
   * This considers the state of the renderer and update
   * actor visibilities accordingly
   */
  void UpdateActorsVisibility();

  //@{
  /**
   * Set/Get up vector
   */
  vtkGetVector3Macro(UpVector, double);
  vtkSetVector3Macro(UpVector, double);
  //@}

  //@{
  /**
   * Set/Get right vector
   */
  vtkGetVector3Macro(RightVector, double);
  vtkSetVector3Macro(RightVector, double);
  //@}

protected:
  vtkF3DRenderer() = default;
  ~vtkF3DRenderer() override = default;

  void ReleaseGraphicsResources(vtkWindow* w) override;
  void UpdateScalarBarVisibility();

  bool IsBackgroundDark();

  void UpdateCheatSheet();

  std::string GenerateMetaDataDescription();

  F3DOptions Options;

  vtkNew<vtkActor> GridActor;
  vtkSmartPointer<vtkActor2D> ScalarBarActor;
  vtkSmartPointer<vtkActor> GeometryActor;
  vtkSmartPointer<vtkActor> PointSpritesActor;
  vtkSmartPointer<vtkVolume> VolumeProp;
  vtkSmartPointer<vtkMapper> PolyDataMapper;
  vtkSmartPointer<vtkMapper> PointGaussianMapper;
  vtkSmartPointer<vtkSmartVolumeMapper> VolumeMapper;
  vtkNew<vtkSkybox> Skybox;
  vtkNew<vtkCamera> InitialCamera;

  vtkSmartPointer<vtkOrientationMarkerWidget> AxisWidget;

  vtkNew<vtkCornerAnnotation> FilenameActor;
  vtkNew<vtkCornerAnnotation> MetaDataActor;
  vtkNew<vtkCornerAnnotation> CheatSheetActor;
  bool CheatSheetNeedUpdate = false;
  bool ScalarsAvailable = false;

  // vtkCornerAnnotation building is too slow for the timer
  vtkNew<vtkTextActor> TimerActor;
  unsigned int Timer = 0;

  bool GridVisible = false;
  bool AxisVisible = false;
  bool EdgesVisible = false;
  bool TimerVisible = false;
  bool FilenameVisible = false;
  bool MetaDataVisible = false;
  bool ScalarBarVisible = false;
  bool ScalarsVisible = false;
  bool CheatSheetVisible = false;
  bool UseRaytracing = false;
  bool UseRaytracingDenoiser = false;
  bool UseDepthPeelingPass = false;
  bool UseFXAAPass = false;
  bool UseSSAOPass = false;
  bool UsePointSprites = false;
  bool UseToneMappingPass = false;
  bool UseVolume = false;
  bool UseInverseOpacityFunction = false;
  bool UseBlurBackground = false;
  bool UseTrackball = false;

  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };
  int UpIndex = 1;
};

#endif
