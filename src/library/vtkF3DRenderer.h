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
   * Set different actors parameters
   */
  void SetFontFile(const std::string& fontFile);
  void SetHDRIFile(const std::string& hdriFile);
  void SetBackgroundColor(const double* backgroundColor);
  //@}

  //@{
  /**
   * Set/Get usages and configurations of different render passes
   */
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetUseFXAAPass(bool use);
  void SetUseToneMappingPass(bool use);
  void SetUseBlurBackground(bool use);
  void SetUseTrackball(bool use);
  void SetRaytracingSamples(int samples);
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
  virtual void Initialize(const f3d::options& options, const std::string& fileInfo, const std::string& up);

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
   * Update the different render passes which can be needed when changing some options
   */
  void UpdateRenderPasses();

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
   * Get useful scene state description. Currently contains only camera state.
   */
  virtual std::string GetSceneDescription();

  /**
   * Return description about the current rendering status
   * Currently contains only information about the grid size if any
   */
  virtual std::string GetRenderingDescription();

  /**
   * Get up vector
   */
  vtkGetVector3Macro(UpVector, double);

  /**
   * Set/Get right vector
   */
  vtkGetVector3Macro(RightVector, double);

  /**
   * Update the actors using the configuration of the renderer
   */
  virtual void UpdateInternalActors();

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
  bool EdgeVisible = false;
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

  int RaytracingSamples = 0;
  int UpIndex = 1;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };

  std::string HDRIFile;
  std::string GridInfo;

  std::string FileInfo;
  std::string Up;
  double BackgroundColor[3];
  std::string FontFile;

  bool HDRINeedUpdate = false;
  bool RenderPassesNeedUpdate = false;
  bool HasHDRI = false;
};

#endif
