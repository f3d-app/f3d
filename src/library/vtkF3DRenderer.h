/**
 * @class   vtkF3DRenderer
 * @brief   A F3D dedicated renderer
 *
 * This renderers all the generic actors added by F3D which includes
 * axis, grid, edges, timer, filename, metadata and cheatsheet.
 * It also handles the different rendering passes, including
 * raytracing, ssao, fxaa, tonemapping.
 */

#ifndef vtkF3DRenderer_h
#define vtkF3DRenderer_h

#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkSkybox.h>

namespace f3d
{
class options;
}
class vtkCornerAnnotation;
class vtkTextActor;

class vtkF3DRenderer : public vtkOpenGLRenderer
{
public:
  static vtkF3DRenderer* New();
  vtkTypeMacro(vtkF3DRenderer, vtkOpenGLRenderer);

  //@{
  /**
   * Set/Get visibility of different actors
   */
  void ShowAxis(bool show);
  void ShowGrid(bool show);
  void ShowEdge(bool show);
  void ShowTimer(bool show);
  void ShowMetaData(bool show);
  void ShowFilename(bool show);
  void ShowCheatSheet(bool show);
  bool IsAxisVisible();
  bool IsGridVisible();
  bool IsEdgeVisible();
  bool IsTimerVisible();
  bool IsFilenameVisible();
  bool IsMetaDataVisible();
  bool IsCheatSheetVisible();
  //@}

  //@{
  /**
   * Set/Get usages of different render passes
   */
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetUseFXAAPass(bool use);
  void SetUseToneMappingPass(bool use);
  void SetUseBlurBackground(bool use);
  void SetUseTrackball(bool use);
  bool UsingRaytracing();
  bool UsingRaytracingDenoiser();
  bool UsingDepthPeelingPass();
  bool UsingSSAOPass();
  bool UsingFXAAPass();
  bool UsingToneMappingPass();
  bool UsingBlurBackground();
  bool UsingTrackball();
  //@}

  /**
   * Reimplemented to handle cheat sheet and timer
   */
  void Render() override;

  /**
   * Update internal options using provided options
   */
  virtual void UpdateOptions(const f3d::options& options);

  /**
   * Initialize the renderer to be used with provided options and file.
   * Should be called after being added to a vtkRenderWindow.
   */
  virtual void Initialize(const f3d::options& options, const std::string& fileInfo);

  //@{
  /**
   * Set/Get the axis widget
   */
  vtkGetSmartPointerMacro(AxisWidget, vtkOrientationMarkerWidget);
  vtkSetSmartPointerMacro(AxisWidget, vtkOrientationMarkerWidget);
  //@}

  /**
   * Get the OpenGL skybox
   */
  vtkGetObjectMacro(Skybox, vtkSkybox);

  /**
   * Set the visibility of the different actors
   * as they were set by the options during the initialization.
   * Also call UpdateInternalActors
   */
  void ShowOptions();

  /**
   * Setup the different render passes
   * as they were set by the options during the initialization.
   */
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

  /**
   * Override to update internal actors that display data
   * in a specific way
   */
  virtual void UpdateInternalActors(){};

  /**
   * Return info about the current rendering array
   * Output info about the grid size f any
   */
  virtual std::string GetRenderingInfo();

protected:
  vtkF3DRenderer();
  ~vtkF3DRenderer() override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  bool IsBackgroundDark();

  /**
   * Update the text of the cheatsheet and mark it for rendering
   */
  void UpdateCheatSheet();

  /**
   * Add related hotkeys options to the cheatsheet.
   * Override to add other hotkeys
   */
  virtual void FillCheatSheetHotkeys(std::stringstream& sheet);

  /**
   * Override to generate a data description
   */
  virtual std::string GenerateMetaDataDescription();

  vtkNew<vtkActor> GridActor;

  vtkNew<vtkSkybox> Skybox;
  vtkNew<vtkCamera> InitialCamera;

  vtkSmartPointer<vtkOrientationMarkerWidget> AxisWidget;

  vtkNew<vtkCornerAnnotation> FilenameActor;
  vtkNew<vtkCornerAnnotation> MetaDataActor;
  vtkNew<vtkCornerAnnotation> CheatSheetActor;
  bool CheatSheetNeedUpdate = false;

  // vtkCornerAnnotation building is too slow for the timer
  vtkNew<vtkTextActor> TimerActor;
  unsigned int Timer = 0;

  bool GridVisible = false;
  bool AxisVisible = false;
  bool EdgesVisible = false;
  bool TimerVisible = false;
  bool FilenameVisible = false;
  bool MetaDataVisible = false;
  bool CheatSheetVisible = false;
  bool UseRaytracing = false;
  bool UseRaytracingDenoiser = false;
  bool UseDepthPeelingPass = false;
  bool UseFXAAPass = false;
  bool UseSSAOPass = false;
  bool UseToneMappingPass = false;
  bool UseBlurBackground = false;
  bool UseTrackball = false;
  bool Verbose = false;

  int RaytracingSamples = 0;
  int UpIndex = 1;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };

  std::string HDRIFile;

  std::string GridInfo;
};

#endif
