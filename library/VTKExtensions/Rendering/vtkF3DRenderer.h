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

#include <vtkLight.h>
#include <vtkOpenGLRenderer.h>

#include <map>

class vtkCornerAnnotation;
class vtkOrientationMarkerWidget;
class vtkSkybox;
class vtkTextActor;

class vtkF3DRenderer : public vtkOpenGLRenderer
{
public:
  static vtkF3DRenderer* New();
  vtkTypeMacro(vtkF3DRenderer, vtkOpenGLRenderer);

  ///@{
  /**
   * Set visibility of different actors
   */
  void ShowAxis(bool show);
  void ShowGrid(bool show);
  void ShowEdge(bool show);
  void ShowTimer(bool show);
  void ShowMetaData(bool show);
  void ShowFilename(bool show);
  void ShowCheatSheet(bool show);
  ///@}

  using vtkOpenGLRenderer::SetBackground;
  ///@{
  /**
   * Set different actors parameters
   */
  void SetLineWidth(double lineWidth);
  virtual void SetPointSize(double pointSize);
  void SetFontFile(const std::string& fontFile);
  void SetHDRIFile(const std::string& hdriFile);
  void SetBackground(const double* backgroundColor) override;
  void SetLightIntensity(const double intensity);
  ///@}

  ///@{
  /**
   * Set usages and configurations of different render passes
   */
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetUseFXAAPass(bool use);
  void SetUseToneMappingPass(bool use);
  void SetUseBlurBackground(bool use);
  void SetRaytracingSamples(int samples);
  ///@}

  ///@{
  /**
   * Set/Get UseTrackball
   */
  void SetUseTrackball(bool use);
  vtkGetMacro(UseTrackball, bool);
  ///@}

  /**
   * Reimplemented to handle cheat sheet and timer
   */
  void Render() override;

  /**
   * Initialize the renderer actors and flags.
   * Should be called after being added to a vtkRenderWindow.
   */
  virtual void Initialize(const std::string& fileInfo, const std::string& up);

  /**
   * Get the OpenGL skybox
   */
  vtkGetObjectMacro(Skybox, vtkSkybox);

  /**
   * Setup the different render passes
   */
  void SetupRenderPasses();

  /**
   * Return description about the current rendering status
   * Currently contains information about the camera and the grid if any
   */
  virtual std::string GetSceneDescription();

  /**
   * Get up vector
   */
  vtkGetVector3Macro(UpVector, double);

  /**
   * Set/Get right vector
   */
  vtkGetVector3Macro(RightVector, double);

protected:
  vtkF3DRenderer();
  ~vtkF3DRenderer() override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  bool IsBackgroundDark();
  void UpdateTextColor();

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

  bool GridInitialized = false;
  int RaytracingSamples = 0;
  int UpIndex = 1;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };

  bool HasHDRI = false;
  std::string HDRIFile;
  std::string FontFile;

  std::map<vtkLight*, double> OriginalLightIntensities;

  std::string CurrentGridInfo;
  std::string GridInfo;
};

#endif
