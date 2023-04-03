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
  void ShowGrid(bool show, double unitSquare = 0, int subdivisions = 10);
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
  void SetFilenameInfo(const std::string& info);
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
  void SetBlurCircleOfConfusionRadius(double radius);
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
   * Reimplemented to handle light creation when no lights are added
   * return the number of lights in the renderer.
   */
  int UpdateLights() override;

  /**
   * Initialize the renderer actors and flags.
   * Should be called after being added to a vtkRenderWindow.
   */
  virtual void Initialize(const std::string& up);

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

  /**
   * Set cache path
   */
  vtkSetMacro(CachePath, std::string);

protected:
  vtkF3DRenderer();
  ~vtkF3DRenderer() override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  bool IsBackgroundDark();
  void UpdateTextColor();

  /**
   * Configure all actors properties according to what has been set for:
   * - point size
   * - line width
   * - show edges
   */
  void ConfigureActorsProperties();

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
  bool ActorsPropertiesConfigured = false;

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
  double CircleOfConfusionRadius = 20.0;

  double PointSize = 10.0;
  double LineWidth = 1.0;

  bool HasHDRI = false;
  std::string HDRIFile;
  std::string FontFile;

  double LightIntensity = 1.0;
  std::map<vtkLight*, double> OriginalLightIntensities;

  std::string CurrentGridInfo;
  std::string GridInfo;

  std::string CachePath;
};

#endif
