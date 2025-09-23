/**
 * @class   vtkF3DRenderer
 * @brief   A F3D dedicated renderer
 *
 * This renderers all the generic actors added by F3D which includes
 * UI, axis, grid, edges, timer, metadata and cheatsheet.
 * It also handles the different rendering passes, including
 * raytracing, ssao, anti-aliasing, tonemapping.
 */

#ifndef vtkF3DRenderer_h
#define vtkF3DRenderer_h

#include "vtkF3DMetaImporter.h"
#include "vtkF3DUIActor.h"

#include <vtkLight.h>
#include <vtkOpenGLRenderer.h>
#include <vtkVersion.h>

#include <filesystem>
#include <map>
#include <optional>

namespace fs = std::filesystem;

class vtkDiscretizableColorTransferFunction;
class vtkColorTransferFunction;
class vtkCornerAnnotation;
class vtkGridAxesActor3D;
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

  /**
   * Enum listing possible anti aliasing modes.
   */
  enum class AntiAliasingMode : unsigned char
  {
    NONE,
    FXAA,
    SSAA
  };

  ///@{
  /**
   * Set visibility of different actors
   */
  void ShowAxis(bool show);
  void ShowGrid(bool show);
  void ShowAxesGrid(bool show);
  void ShowEdge(const std::optional<bool>& show);
  void ShowTimer(bool show);
  void ShowMetaData(bool show);
  void ShowFilename(bool show);
  void ShowCheatSheet(bool show);
  void ShowConsole(bool show);
  void ShowMinimalConsole(bool show);
  void ShowDropZone(bool show);
  void ShowDropZoneLogo(bool show);
  void ShowHDRISkybox(bool show);
  void ShowArmature(bool show);
  ///@}

  using vtkOpenGLRenderer::SetBackground;
  ///@{
  /**
   * Set different actors parameters
   */
  void SetLineWidth(const std::optional<double>& lineWidth);
  void SetPointSize(const std::optional<double>& pointSize);
  void SetFontFile(const std::optional<fs::path>& fontFile);
  void SetFontScale(const double fontScale);
  void SetHDRIFile(const std::optional<fs::path>& hdriFile);
  void SetUseImageBasedLighting(bool use) override;
  void SetBackground(const double* backgroundColor) override;
  void SetLightIntensity(const double intensity);
  void SetFilenameInfo(const std::string& info);
  void SetDropZoneInfo(const std::string& info);
  void SetDropZoneBinds(const std::vector<std::pair<std::string, std::string>>& dropZoneBinds);
  void SetGridAbsolute(bool absolute);
  void SetGridUnitSquare(const std::optional<double>& unitSquare);
  void SetGridSubdivisions(int subdivisions);
  void SetGridColor(const std::vector<double>& color);
  void SetBackdropOpacity(const double backdropOpacity);
  ///@}

  ///@{
  /**
   * Set usages and configurations of different render passes
   */
  void SetUseRaytracing(bool use);
  void SetUseRaytracingDenoiser(bool use);
  void SetUseDepthPeelingPass(bool use);
  void SetUseSSAOPass(bool use);
  void SetAntiAliasingMode(AntiAliasingMode mode);
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
   * Initialize actors properties related to the up vector using the provided upString, including
   * the camera
   */
  void InitializeUpVector(const std::vector<double>& upVec);

  /**
   * Compute bounds of visible props as transformed by given matrix.
   */
  vtkBoundingBox ComputeVisiblePropOrientedBounds(const vtkMatrix4x4*);

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
   * Set the index of refraction of the base layer on all actors
   */
  void SetBaseIOR(const std::optional<double>& baseIOR);

  /**
   * Set the surface color on all actors
   */
  void SetSurfaceColor(const std::optional<std::vector<double>>& color);

  /**
   * Set the emmissive factors on all actors
   */
  void SetEmissiveFactor(const std::optional<std::vector<double>>& factors);

  /**
   * Set the texture transform on all actors
   */
  void SetTexturesTransform(const std::optional<std::vector<double>>& transform);

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
  void SetTextureMatCap(const std::optional<fs::path>& tex);

  /**
   * Set the base color texture on all actors
   */
  void SetTextureBaseColor(const std::optional<fs::path>& tex);

  /**
   * Set the material texture on all actors
   */
  void SetTextureMaterial(const std::optional<fs::path>& tex);

  /**
   * Set the emissive texture on all actors
   */
  void SetTextureEmissive(const std::optional<fs::path>& tex);

  /**
   * Set the normal texture on all actors
   */
  void SetTextureNormal(const std::optional<fs::path>& tex);

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
   * Set Colormap Discretization
   * Defaults to std::nullopt which is no discretization.
   */
  void SetColormapDiscretization(std::optional<int> discretization);

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
   * Cycle the current array for coloring, actually setting EnableColoring and ArrayNameForColoring
   * members. This loops back to not coloring if volume is not enabled. This can trigger
   * CycleComponentForColoring if current component is not valid.
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
   * Configure the cheatsheet data from the provided info
   * Should be called before Render() if CheatSheetInfoNeedsUpdate() returns true.
   */
  void ConfigureCheatSheet(const std::vector<vtkF3DUIActor::CheatSheetGroup>& info);

  /**
   * Use this method to flag in the renderer that the cheatsheet needs to be updated
   * This is not required to call when using any of the setter of the renderer
   */
  void SetCheatSheetConfigured(bool flag);

  /**
   * Set the UI delta time (time between frame being rendered) in seconds
   */
  void SetUIDeltaTime(double time);

  /**
   * Set console badge enabled status
   */
  void SetConsoleBadgeEnabled(bool enabled);

private:
  vtkF3DRenderer();
  ~vtkF3DRenderer() override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

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
   * Configure the Grid Axes actor
   */
  void ConfigureGridAxesUsingCurrentActors();

  /**
   * Configure the different render passes
   */
  void ConfigureRenderPasses();

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

  /**
   * Convenience method to set texture transform in ConfigureActorsProperties()
   */
  void ConfigureActorTextureTransform(vtkActor* actorBase, const double* matrix);

  vtkSmartPointer<vtkOrientationMarkerWidget> AxisWidget;

  // Does vtk version support GridAxesActor
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250513)
  vtkNew<vtkGridAxesActor3D> GridAxesActor;
#endif

  vtkNew<vtkActor> GridActor;
  vtkNew<vtkSkybox> SkyboxActor;
  vtkNew<vtkF3DUIActor> UIActor;

  unsigned int Timer = 0; // Timer OpenGL query

  bool CheatSheetConfigured = false;
  bool ActorsPropertiesConfigured = false;
  bool GridConfigured = false;
  bool GridAxesConfigured = false;
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
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250513)
  bool AxesGridVisible = false;
#endif
  std::optional<bool> EdgeVisible;
  bool TimerVisible = false;
  bool FilenameVisible = false;
  bool MetaDataVisible = false;
  bool CheatSheetVisible = false;
  bool ConsoleVisible = false;
  bool MinimalConsoleVisible = false;
  bool DropZoneVisible = false;
  bool DropZoneLogoVisible = false;
  bool HDRISkyboxVisible = false;
  bool ArmatureVisible = false;
  bool UseRaytracing = false;
  bool UseRaytracingDenoiser = false;
  bool UseDepthPeelingPass = false;
  AntiAliasingMode AntiAliasingModeEnabled = AntiAliasingMode::NONE;
  bool UseSSAOPass = false;
  bool UseToneMappingPass = false;
  bool UseBlurBackground = false;
  std::optional<bool> UseOrthographicProjection = false;
  bool UseTrackball = false;
  bool InvertZoom = false;

  int RaytracingSamples = 0;
  double UpVector[3] = { 0.0, 1.0, 0.0 };
  double RightVector[3] = { 1.0, 0.0, 0.0 };
  double CircleOfConfusionRadius = 20.0;
  std::optional<double> PointSize;
  std::optional<double> LineWidth;
  std::optional<double> GridUnitSquare;
  int GridSubdivisions = 10;
  double GridColor[3] = { 0.0, 0.0, 0.0 };

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

  std::optional<fs::path> FontFile;
  double FontScale = 1.0;

  double LightIntensity = 1.0;
  std::map<vtkLight*, double> OriginalLightIntensities;

  std::string CurrentGridInfo;
  std::string GridInfo;

  std::string CachePath;

  std::optional<std::string> BackfaceType;
  std::optional<std::string> FinalShader;

  vtkF3DMetaImporter* Importer = nullptr;
  vtkMTimeType ImporterTimeStamp = 0;
  vtkMTimeType ImporterUpdateTimeStamp = 0;

  vtkNew<vtkScalarBarActor> ScalarBarActor;
  bool ScalarBarActorConfigured = false;

  bool ColoringMappersConfigured = false;
  bool PointSpritesMappersConfigured = false;
  bool VolumePropsAndMappersConfigured = false;
  bool ColoringConfigured = false;

  std::optional<double> Opacity;
  std::optional<double> Roughness;
  std::optional<double> Metallic;
  std::optional<double> BaseIOR;
  std::optional<double> NormalScale;
  std::optional<std::vector<double>> SurfaceColor;
  std::optional<std::vector<double>> EmissiveFactor;
  std::optional<std::vector<double>> TexturesTransform;
  std::optional<fs::path> TextureMatCap;
  std::optional<fs::path> TextureBaseColor;
  std::optional<fs::path> TextureMaterial;
  std::optional<fs::path> TextureEmissive;
  std::optional<fs::path> TextureNormal;

  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ColorTransferFunction;
  bool ExpandingRangeSet = false;
  bool UsingExpandingRange = true;
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
  std::optional<int> ColormapDiscretization;
};

#endif
