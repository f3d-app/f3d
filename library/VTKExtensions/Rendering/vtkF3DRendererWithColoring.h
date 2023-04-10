/**
 * @class   vtkF3DRendererWithColoring
 * @brief   A specialiwed renderer that can handle coloring
 *
 * A specialization of vtkF3DRenderer that can handle coloring
 * with actors such as a geometry actor, a point sprites actor,
 * a volume mapper. It also handle the showing of the related scalar bar.
 */

#ifndef vtkF3DRendererWithColoring_h
#define vtkF3DRendererWithColoring_h

#include "vtkF3DRenderer.h"

#include "vtkF3DGenericImporter.h"

#include <vtkWeakPointer.h>

class vtkColorTransferFunction;
class vtkDataArray;
class vtkDataSetAttributes;
class vtkScalarBarActor;

class vtkF3DRendererWithColoring : public vtkF3DRenderer
{
public:
  static vtkF3DRendererWithColoring* New();
  vtkTypeMacro(vtkF3DRendererWithColoring, vtkF3DRenderer);

  /**
   * Initialize all actors and flags
   */
  void Initialize(const std::string& up) override;

  /**
   * Set the roughness on all coloring actors
   */
  void SetRoughness(double roughness);

  /**
   * Set the surface color on all coloring actors
   */
  void SetSurfaceColor(double* color);

  /**
   * Set the emmissive factors on all coloring actors
   */
  void SetEmissiveFactor(double* factors);

  /**
   * Set the opacity on all coloring actors
   */
  void SetOpacity(double opacity);

  /**
   * Set the metallic on all coloring actors
   */
  void SetMetallic(double metallic);

  /**
   * Set the normal scale on all coloring actors
   */
  void SetNormalScale(double normalScale);

  /**
   * Set the base color texture on all coloring actors
   */
  void SetTextureBaseColor(const std::string& tex);

  /**
   * Set the metarial texture on all coloring actors
   */
  void SetTextureMaterial(const std::string& tex);

  /**
   * Set the emissive texture on all coloring actors
   */
  void SetTextureEmissive(const std::string& tex);

  /**
   * Set the normal texture on all coloring actors
   */
  void SetTextureNormal(const std::string& tex);

  /**
   * Set the pointSize on the pointGaussianMapper as well as calls superclass implementation.
   */
  void SetPointSize(double pointSize) override;

  /**
   * Set the visibility of the scalar bar.
   * It will only be shown when coloring and not
   * using direct scalars rendering.
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
  void SetScalarBarRange(const std::vector<double>& range);

  /**
   * Set the colormap to use
   * Setting an empty vector will use default color map
   */
  void SetColormap(const std::vector<double>& colormap);

  enum class CycleType
  {
    NONE,
    FIELD,
    ARRAY_INDEX,
    COMPONENT
  };

  enum class ColoringStyle_t
  {
    NONE,
    DIRECT,
    MAGNITUDE,
    COMPONENT
  };

  enum class ColoringField_t
  {
    POINT,
    CELL
  };

  /**
   * Cycle the shown scalars according to the cycle type
   */
  void CycleScalars(CycleType type);

  /**
   * Set the generic importer to handle coloring and other actors related actions with.
   */
  void SetImporter(vtkF3DGenericImporter* importer);

  /**
   * Set the coloring, call after SetColoringAttributes.
   * This method will try to find the corresponding array in the coloring attributes and will
   * position ArrayIndexForColoring and DataForColoring accordingly.
   */
  void SetColoring(const ColoringStyle_t& coloringStyle, const ColoringField_t& coloringField, const std::string& coloringArrayName, int coloringComponent);

  ///@{
  /**
   * Get current coloring information,
   * Useful after using Cycle methods
   */
  bool GetColoringUseCell();
  std::string GetColoringArrayName();
  int GetColoringComponent();
  ///@}

  /**
   * Reimplemented to update the visibility and coloring of internal actors
   * as well as the scalar bar actors.
   * Call Superclass::UpdateActors at the end.
   */
  void UpdateActors() override;

  /**
   * Get information about the current coloring
   */
  virtual std::string GetColoringDescription();

protected:
  vtkF3DRendererWithColoring() = default;
  ~vtkF3DRendererWithColoring() override = default;

  /**
   * Configure all coloring actors properties:
   *  - roughness
   *
   */
  void ConfigureColoringActorsProperties();

  /**
   * Configure coloring for all coloring actors
   */
  void ConfigureColoring();

  /**
   * Convenience method for configuring a poly data mapper for coloring
   */
  static void ConfigureMapperForColoring(vtkPolyDataMapper* mapper, vtkDataArray* array,
    int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag = false);

  /**
   * Convenience method for configuring a volume mapper and volume for coloring
   */
  static void ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper, vtkVolume* volume,
    vtkDataArray* array, int component, vtkColorTransferFunction* ctf, double range[2],
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
  void ConfigureRangeAndCTFForColoring(const vtkF3DGenericImporter::ColoringInfo& info);

  /**
   * Fill cheatsheet hotkeys string stream
   */
  void FillCheatSheetHotkeys(std::stringstream& sheet) override;

  /**
   * Switch between point data and cell data coloring
   */
  void CycleFieldForColoring();

  /**
   * Increment the array index or loop it back
   * When not using volume, it will loop back
   * to not coloring
   */
  void CycleArrayIndexForColoring();

  /**
   * Cycle the component in used for rendering
   * looping back to direct scalars
   */
  void CycleComponentForColoring();

  /**
   * Check coloring is currently valid and return a cycle type to perform if not
   */
  CycleType CheckColoring();

  /**
   * Generate a padded metadata description
   * using the internal importer
   */
  std::string GenerateMetaDataDescription() override;

  /**
   * Convert a component index into a string
   * If there is a component name defined in the current coloring information, display it.
   * Otherwise, use component #index as the default value.
   */
  //std::string ComponentToString(int component);
  std::string GetColoringComponentName();

  /**
   * Shorten a provided name with "..."
   */
  static std::string ShortName(const std::string& name, int component);

  vtkWeakPointer<vtkF3DGenericImporter> Importer = nullptr;

  vtkNew<vtkScalarBarActor> ScalarBarActor;
  bool ScalarBarActorConfigured = false;

  bool GeometryMappersConfigured = false;
  bool PointSpritesMappersConfigured = false;
  bool VolumePropsAndMappersConfigured = false;
  bool ColoringActorsPropertiesConfigured = false;
  bool ColoringConfigured = false;

  double Opacity = 1.;
  double Roughness = 0.3;
  double Metallic = 0.;
  double NormalScale = 1.;
  double SurfaceColor[3] = { 1., 1., 1. };
  double EmissiveFactor[3] = { 1., 1., 1. };
  std::string TextureBaseColor;
  std::string TextureMaterial;
  std::string TextureEmissive;
  std::string TextureNormal;

  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
  double ColorRange[2];
  bool ColorTransferFunctionConfigured = false;

  bool ScalarBarVisible = false;
  bool UsePointSprites = false;
  bool UseVolume = false;
  bool UseInverseOpacityFunction = false;

  std::vector<double> UserScalarBarRange;
  std::vector<double> Colormap;

  vtkMTimeType ImporterTimeStamp = 0;

  ColoringStyle_t ColoringStyle = ColoringStyle_t::MAGNITUDE;
  ColoringField_t ColoringField = ColoringField_t::POINT;
  std::string ColoringArrayName;
  int ColoringArrayIndex = -1;
  int ColoringComponent = 0;
};

#endif
