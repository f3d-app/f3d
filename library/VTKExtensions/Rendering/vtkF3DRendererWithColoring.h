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
    F3D_CYCLE_NONE,
    F3D_CYCLE_FIELD,
    F3D_CYCLE_ARRAY_INDEX,
    F3D_CYCLE_COMPONENT
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
  void SetColoring(bool useCellData, const std::string& arrayName, int component);

  ///@{
  /**
   * Get current coloring infomations,
   * Useful after using Cycle methods
   */
  bool GetColoringUseCell();
  std::string GetColoringArrayName();
  int GetColoringComponent();
  ///@}

  /**
   * Update the visibility and coloring of internal actors as well as the scalar bar actors
   */
  void UpdateColoringActors();

  /**
   * Get information about the current coloring
   */
  virtual std::string GetColoringDescription();

protected:
  vtkF3DRendererWithColoring() = default;
  ~vtkF3DRendererWithColoring() override = default;

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
  void ConfigureScalarBarActorForColoring(vtkScalarBarActor* scalarBar,
    const std::string& arrayName, int component, vtkColorTransferFunction* ctf);

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
  std::string ComponentToString(int component);

  /**
   * Shorten a provided name with "..."
   */
  static std::string ShortName(const std::string& name, int component);

  vtkF3DGenericImporter* Importer = nullptr;

  vtkNew<vtkScalarBarActor> ScalarBarActor;
  bool ScalarBarActorConfigured = false;

  bool PolyDataMapperConfigured = false;
  bool PointGaussianMapperConfigured = false;
  bool VolumeConfigured = false;

  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
  double ColorRange[2];
  bool ColorTransferFunctionConfigured = false;

  bool UseCellColoring = false;
  int ArrayIndexForColoring = -1;
  int ComponentForColoring = -1;

  bool ScalarBarVisible = false;
  bool UsePointSprites = false;
  bool UseVolume = false;
  bool UseInverseOpacityFunction = false;

  std::vector<double> UserScalarBarRange;
  std::vector<double> Colormap;

  vtkTimeStamp ConfigurationTimeStamp;
  vtkTimeStamp ColoringTimeStamp;
  vtkMTimeType ColoringUpdateTime = 0;
};

#endif
