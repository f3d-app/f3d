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

#include "vtkF3DMetaImporter.h"

#include <vtkLight.h>
#include <vtkOpenGLRenderer.h>

#include <map>
#include <optional>

class vtkColorTransferFunction;
class vtkCornerAnnotation;
class vtkF3DDropZoneActor;
class vtkImageReader2;
class vtkOrientationMarkerWidget;
class vtkScalarBarActor;
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
  void ShowEdge(const std::optional<bool>& show);
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
  void SetLineWidth(const std::optional<double>& lineWidth);
  void SetPointSize(const std::optional<double>& pointSize);
  void SetFontFile(const std::optional<std::string>& fontFile);
  void SetHDRIFile(const std::optional<std::string>& hdriFile);
  void SetUseImageBasedLighting(bool use) override;
  void SetBackground(const double* backgroundColor) override;
  void SetLightIntensity(const double intensity);
  void SetFilenameInfo(const std::string& info);
  void SetDropZoneInfo(const std::string& info);
  void SetGridAbsolute(bool absolute);
  void SetGridUnitSquare(const std::optional<double>& unitSquare);
  void SetGridSubdivisions(int subdivisions);
  void SetGridColor(const std::vector<double>& color);
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
  void SetBackfaceType(const std::optional<std::string>& backfaceType);
  void SetFinalShader(const std::optional<std::string>& finalShader);
  ///@}

  /**
   * Set SetUseOrthographicProjection
   */
  void SetUseOrthographicProjection(const std::optional<bool>& use);

  ///@{
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
   *  - Timer
   * before actual rendering, only when needed
   */
  void Render() override;

  /**
   * Reimplemented to account for grid actor
   */
  void ResetCameraClippingRange() override;

  /**
   * Set properties on each imported actors and also configure the coloring
   * Then update dedicated actors and logics according to the properties of this class:
   *  - HDRI
   *  - MetaData
   *  - Texts
   *  - RenderPasses
   *  - Grid
   */
  void UpdateActors();

  /**
   * Reimplemented to handle light creation when no lights are added
   * return the number of lights in the renderer.
   */
  int UpdateLights() override;

  /**
   * Initialize the renderer actors and flags.
   * Should be called after being added to a vtkRenderWindow.
   */
  void Initialize();

  /**
   * Initialize actors properties related to the up vector using the provided upString, including the camera
   */
  void InitializeUpVector(const std::string& upString);

  /**
   * Get the OpenGL skybox
   */
  vtkGetObjectMacro(SkyboxActor, vtkSkybox);

  /**
   * Return description about the current rendering status
   * Currently contains information about the camera and the grid if any
   * Returns a multiline string containing the scene description
   */
  std::string GetSceneDescription();

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

  /**
   * Set the roughness on all actors
   */
  void SetRoughness(const std::optional<double>& roughness);

  /**
   * Set the surface color on all actors
   */
  void SetSurfaceColor(const std::optional<std::vector<double>>& color);

  /**
   * Set the emmissive factors on all actors
   */
  void SetEmissiveFactor(const std::optional<std::vector<double>>& factors);

  /**
   * Set the opacity on all actors
   */
  void SetOpacity(const std::optional<double>& opacity);

  /**
   * Set the metallic on all actors
   */
  void SetMetallic(const std::optional<double>& metallic);

  /**
   * Set the normal scale on all actors
   */
  void SetNormalScale(const std::optional<double>& normalScale);

  /**
   * Set the material capture texture on all actors.
   * This texture includes baked lighting effect,
   * so all other material textures are ignored.
   */
  void SetTextureMatCap(const std::optional<std::string>& tex);

  /**
   * Set the base color texture on all actors
   */
  void SetTextureBaseColor(const std::optional<std::string>& tex);

  /**
   * Set the material texture on all actors
   */
  void SetTextureMaterial(const std::optional<std::string>& tex);

  /**
   * Set the emissive texture on all actors
   */
  void SetTextureEmissive(const std::optional<std::string>& tex);

  /**
   * Set the normal texture on all actors
   */
  void SetTextureNormal(const std::optional<std::string>& tex);

  enum class SplatType
  {
    SPHERE,
    GAUSSIAN
  };

  /**
   * Set the point sprites size and the splat type on the pointGaussianMapper
   */
  void SetPointSpritesProperties(SplatType splatType, double pointSpritesSize);

  /**
   * Set the visibility of the scalar bar.
   * It will only be shown when coloring and not shown
   * when using direct scalars rendering.
   */
  void ShowScalarBar(bool show);

  /**
   * Set the visibility of the point sprites actor.
   * It will only be shown if raytracing and volume are not enabled
   */
  void SetUsePointSprites(bool use);

  /**
   * Set the visibility of the volume actor.
   * It will only be shown if the data is compatible with volume rendering
   * and raytracing is not enabled
   */
  void SetUseVolume(bool use);

  /**
   * Set the use of an inverted opacity function
   * for volume rendering..
   */
  void SetUseInverseOpacityFunction(bool use);

  /**
   * Set the range of the scalar bar
   * Setting an empty vector will use automatic range
   */
  void SetScalarBarRange(const std::optional<std::vector<double>>& range);

  /**
   * Set the colormap to use
   * Setting an empty vector will use default color map
   */
  void SetColormap(const std::vector<double>& colormap);

  /**
   * Set the meta importer to recover coloring information from
   */
  void SetImporter(vtkF3DMetaImporter* importer);

  ///@{
  /**
   * Set/Get if coloring is enabled
   */
  void SetEnableColoring(bool enable);
  vtkGetMacro(EnableColoring, bool);
  ///@}

  ///@{
  /**
   * Set/Get if using point or cell data coloring
   */
  void SetUseCellColoring(bool useCell);
  vtkGetMacro(UseCellColoring, bool);
  ///@}

  ///@{
  /**
   * Set/Get the name of the array to use for coloring
   */
  void SetArrayNameForColoring(const std::optional<std::string>& arrayName);
  std::optional<std::string> GetArrayNameForColoring();
  ///@}

  ///@{
  /**
   * Set/Get the name of the component to use for coloring
   */
  void SetComponentForColoring(int component);
  vtkGetMacro(ComponentForColoring, int);
  ///@}

  /**
   * Get information about the current coloring
   * Returns a single line string containing the coloring description
   */
  virtual std::string GetColoringDescription();

  /**
   * Switch between point data and cell data coloring, actually setting UseCellColoring member.
   * This can trigger CycleArrayForColoring if current array is not valid.
   */
  void CycleFieldForColoring();

  /**
   * Cycle the current array for coloring, actually setting EnableColoring and ArrayNameForColoring members.
   * This loops back to not coloring if volume is not enabled.
   * This can trigger CycleComponentForColoring if current component is not valid.
   */
  void CycleArrayForColoring();

  /**
   * Cycle the component in used for rendering
   * looping back to direct scalars
   */
  void CycleComponentForColoring();

  /**
   * Convert a component index into a string
   * If there is a component name defined in the current coloring information, display it.
   * Otherwise, use component #index as the default value.
   */
  std::string ComponentToString(int component);

  /**
   * Return true if the cheatsheet info is potentially
   * out of date since the last ConfigureCheatSheet call,
   * false otherwise.
   */
  bool CheatSheetNeedsUpdate() const;

  /**
   * Configure the cheatsheet text from the provided info
   * Should be called before Render() if CheatSheetInfoNeedsUpdate() returns true.
   */
  void ConfigureCheatSheet(const std::string& info);

  /**
   * Use this method to flag in the renderer that the cheatsheet needs to be updated
   * This is not required to call when using any of the setter of the renderer
   */
  void SetCheatSheetConfigured(bool flag);

private:
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
   * Configure all actors properties
   */
  void ConfigureActorsProperties();

  /**
   * Configure the grid
   */
  void ConfigureGridUsingCurrentActors();

  /**
   * Configure the different render passes
   */
  void ConfigureRenderPasses();

  /**
   * Generate a padded metadata description
   * using the internal importer.
   * Returns a multiline string containing the meta data description
   */
  std::string GenerateMetaDataDescription();

  /**
   * Create a cache directory if a HDRIHash is set
   */
  void CreateCacheDirectory();

  /**
   * Configure coloring for all actors
   */
  void ConfigureColoring();

  /**
   * Convenience method for configuring a poly data mapper for coloring
   * Return true if mapper was configured for coloring, false otherwise.
   */
  static bool ConfigureMapperForColoring(vtkPolyDataMapper* mapper, const std::string& name,
    int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag = false);

  /**
   * Convenience method for configuring a volume mapper and volume prop for coloring
   * Return true if they were configured for coloring, false otherwise.
   */
  static bool ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper, vtkVolume* volume,
    const std::string& name, int component, vtkColorTransferFunction* ctf, double range[2],
    bool cellFlag = false, bool inverseOpacityFlag = false);

  /**
   * Convenience method for configuring a scalar bar actor for coloring
   */
  void ConfigureScalarBarActorForColoring(vtkScalarBarActor* scalarBar, std::string arrayName,
    int component, vtkColorTransferFunction* ctf);

  /**
   * Configure internal range and color transfer function according to provided
   * coloring info
   */
  void ConfigureRangeAndCTFForColoring(const F3DColoringInfoHandler::ColoringInfo& info);

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
  std::optional<bool> EdgeVisible;
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
  std::optional<bool> UseOrthographicProjection = false;
  bool UseTrackball = false;
  bool InvertZoom = false;

  int RaytracingSamples = 0;
  int UpIndex = 1;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };
  double CircleOfConfusionRadius = 20.0;
  std::optional<double> PointSize;
  std::optional<double> LineWidth;
  std::optional<double> GridUnitSquare;
  int GridSubdivisions = 10;
  double GridColor[3] = { 0.0, 0.0, 0.0 };

  std::optional<std::string> HDRIFile;
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

  std::optional<std::string> FontFile;

  double LightIntensity = 1.0;
  std::map<vtkLight*, double> OriginalLightIntensities;

  std::string CurrentGridInfo;
  std::string GridInfo;

  std::string CachePath;

  std::optional <std::string> BackfaceType;
  std::optional <std::string> FinalShader;

  vtkF3DMetaImporter* Importer = nullptr;
  vtkMTimeType ImporterTimeStamp = 0;

  vtkNew<vtkScalarBarActor> ScalarBarActor;
  bool ScalarBarActorConfigured = false;

  bool ColoringMappersConfigured = false;
  bool PointSpritesMappersConfigured = false;
  bool VolumePropsAndMappersConfigured = false;
  bool ColoringConfigured = false;

  std::optional<double> Opacity;
  std::optional<double> Roughness;
  std::optional<double> Metallic;
  std::optional<double> NormalScale;
  std::optional<std::vector<double>> SurfaceColor;
  std::optional<std::vector<double>> EmissiveFactor;
  std::optional<std::string> TextureMatCap;
  std::optional<std::string> TextureBaseColor;
  std::optional<std::string> TextureMaterial;
  std::optional<std::string> TextureEmissive;
  std::optional<std::string> TextureNormal;

  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
  double ColorRange[2] = { 0.0, 1.0 };
  bool ColorTransferFunctionConfigured = false;

  bool EnableColoring = false;
  bool UseCellColoring = false;
  int ComponentForColoring = -1;
  std::optional<std::string> ArrayNameForColoring;

  bool ScalarBarVisible = false;
  bool UsePointSprites = false;
  bool UseVolume = false;
  bool UseInverseOpacityFunction = false;

  std::optional<std::vector<double>> UserScalarBarRange;
  std::vector<double> Colormap;
};

#endif
