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
class vtkF3DDropZoneActor;
class vtkImageReader2;
class vtkOrientationMarkerWidget;
class vtkSkybox;
class vtkTextActor;

class vtkF3DRenderer : public vtkOpenGLRenderer
{
public:
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
  void ShowDropZone(bool show);
  void ShowHDRISkybox(bool show);
  ///@}

  using vtkOpenGLRenderer::SetBackground;
  ///@{
  /**
   * Set different actors parameters
   */
  void SetLineWidth(double lineWidth);
  void SetPointSize(double pointSize);
  void SetFontFile(const std::string& fontFile);
  void SetHDRIFile(const std::string& hdriFile);
  void SetUseImageBasedLighting(bool use) override;
  void SetBackground(const double* backgroundColor) override;
  void SetLightIntensity(const double intensity);
  void SetFilenameInfo(const std::string& info);
  void SetDropZoneInfo(const std::string& info);
  void SetGridAbsolute(bool absolute);
  void SetGridUnitSquare(double unitSquare);
  void SetGridSubdivisions(int subdivisions);
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
   * Set SetUseOrthographicProjection
   */
  void SetUseOrthographicProjection(bool use);
  /**
   * Set/Get UseTrackball
   */
  void SetUseTrackball(bool use);
  vtkGetMacro(UseTrackball, bool);
  ///@}

  ///@{
  /**
   * Set/Get InvertZoom
   */
  vtkSetMacro(InvertZoom, bool);
  vtkGetMacro(InvertZoom, bool);
  ///@}

  /**
   * Reimplemented to configure:
   *  - ActorsProperties
   *  - CheatSheet
   *  - Timer
   * before actual rendering, only when needed
   */
  void Render() override;

  /**
   * Reimplemented to account for grid actor
   */
  void ResetCameraClippingRange() override;

  /**
   * Update actors according to the properties of this class:
   *  - Grid
   */
  virtual void UpdateActors();

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
  vtkGetObjectMacro(SkyboxActor, vtkSkybox);

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
   * Set cache path, only used by the HDRI logic
   */
  void SetCachePath(const std::string& cachePath);

protected:
  vtkF3DRenderer();
  ~vtkF3DRenderer() override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  bool IsBackgroundDark();

  /**
   * Configure meta data actor visibility and content
   */
  void ConfigureMetaData();

  /**
   * Configure text actors properties font file and color
   */
  void ConfigureTextActors();

  ///@{
  /**
   * Configure HDRI actor and related lighting textures
   */
  void ConfigureHDRI();
  void ConfigureHDRIReader();
  void ConfigureHDRIHash();
  void ConfigureHDRITexture();
  void ConfigureHDRILUT();
  void ConfigureHDRISphericalHarmonics();
  void ConfigureHDRISpecular();
  void ConfigureHDRISkybox();
  ///@}

  ///@{
  /**
   * Methods to check if certain HDRI caches are available
   */
  bool CheckForSpecCache(std::string& path);
  bool CheckForSHCache(std::string& path);
  ///@}

  /**
   * Configure all actors properties according to what has been set for:
   * - point size
   * - line width
   * - show edges
   */
  void ConfigureActorsProperties();

  /**
   * Configure the cheatsheet text and mark it for rendering
   */
  void ConfigureCheatSheet();

  /**
   * Configure the grid
   */
  void ConfigureGridUsingCurrentActors();

  /**
   * Configure the different render passes
   */
  void ConfigureRenderPasses();

  /**
   * Add related hotkeys options to the cheatsheet.
   * Override to add other hotkeys
   */
  virtual void FillCheatSheetHotkeys(std::stringstream& sheet);

  /**
   * Override to generate a data description
   */
  virtual std::string GenerateMetaDataDescription() = 0;

  /**
   * Create a cache directory if a HDRIHash is set
   */
  void CreateCacheDirectory();

  vtkNew<vtkCamera> InitialCamera;

  vtkSmartPointer<vtkOrientationMarkerWidget> AxisWidget;

  vtkNew<vtkCornerAnnotation> FilenameActor;
  vtkNew<vtkCornerAnnotation> MetaDataActor;
  vtkNew<vtkCornerAnnotation> CheatSheetActor;
  vtkNew<vtkF3DDropZoneActor> DropZoneActor;
  vtkNew<vtkActor> GridActor;
  vtkNew<vtkSkybox> SkyboxActor;

  // vtkCornerAnnotation building is too slow for the timer
  vtkNew<vtkTextActor> TimerActor;
  unsigned int Timer = 0;

  bool CheatSheetConfigured = false;
  bool ActorsPropertiesConfigured = false;
  bool GridConfigured = false;
  bool RenderPassesConfigured = false;
  bool LightIntensitiesConfigured = false;
  bool TextActorsConfigured = false;
  bool MetaDataConfigured = false;
  bool HDRIReaderConfigured = false;
  bool HDRIHashConfigured = false;
  bool HDRITextureConfigured = false;
  bool HDRILUTConfigured = false;
  bool HDRISphericalHarmonicsConfigured = false;
  bool HDRISpecularConfigured = false;
  bool HDRISkyboxConfigured = false;

  bool GridVisible = false;
  bool GridAbsolute = false;
  bool AxisVisible = false;
  bool EdgeVisible = false;
  bool TimerVisible = false;
  bool FilenameVisible = false;
  bool MetaDataVisible = false;
  bool CheatSheetVisible = false;
  bool DropZoneVisible = false;
  bool HDRISkyboxVisible = false;
  bool UseRaytracing = false;
  bool UseRaytracingDenoiser = false;
  bool UseDepthPeelingPass = false;
  bool UseFXAAPass = false;
  bool UseSSAOPass = false;
  bool UseToneMappingPass = false;
  bool UseBlurBackground = false;
  bool UseOrthographicProjection = false;
  bool UseTrackball = false;
  bool InvertZoom = false;

  int RaytracingSamples = 0;
  int UpIndex = 1;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };
  double CircleOfConfusionRadius = 20.0;
  double PointSize = 10.0;
  double LineWidth = 1.0;
  double GridUnitSquare = 0.0;
  int GridSubdivisions = 10;

  std::string HDRIFile;
  vtkSmartPointer<vtkImageReader2> HDRIReader;
  bool HasValidHDRIReader = false;
  bool UseDefaultHDRI = false;
  std::string HDRIHash;
  bool HasValidHDRIHash = false;
  vtkSmartPointer<vtkTexture> HDRITexture;
  bool HasValidHDRITexture = false;
  bool HasValidHDRILUT = false;
  bool HasValidHDRISH = false;
  bool HasValidHDRISpec = false;

  std::string FontFile;

  double LightIntensity = 1.0;
  std::map<vtkLight*, double> OriginalLightIntensities;

  std::string CurrentGridInfo;
  std::string GridInfo;

  std::string CachePath;
};

#endif
