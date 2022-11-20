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

// Include needed because of the smart pointers macro
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartVolumeMapper.h>

class vtkDataArray;
class vtkColorTransferFunction;
class vtkDataSetAttributes;

class vtkF3DRendererWithColoring : public vtkF3DRenderer
{
public:
  static vtkF3DRendererWithColoring* New();
  vtkTypeMacro(vtkF3DRendererWithColoring, vtkF3DRenderer);

  /**
   * Initialize all actors and flags
   */
  void Initialize(const std::string& fileInfo, const std::string& up) override;

  /**
   * Set the pointSize on the pointGaussianMapper as well as calls superclass implementation.
   */
  void SetPointSize(double pointSize) override;

  ///@{
  /**
   * Set the visibility of the scalar bar.
   * It will only be shown when coloring and not
   * using direct scalars rendering.
   */
  void ShowScalarBar(bool show);
  ///@}

  ///@{
  /**
   * Set the visibility of the point sprites actor.
   * It will only be shown if raytracing and volume are not enabled
   */
  void SetUsePointSprites(bool use);
  ///@}

  ///@{
  /**
   * Set the visibility of the volume actor.
   * It will only be shown if the data is compatible with volume rendering
   * and raytracing is not enabled
   */
  void SetUseVolume(bool use);
  ///@}

  ///@{
  /**
   * Set the use of an inverted opacity function
   * for volume rendering..
   */
  void SetUseInverseOpacityFunction(bool use);
  ///@}

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

  enum CycleTypeEnum
  {
    F3D_FIELD_CYCLE = 0,
    F3D_ARRAY_CYCLE,
    F3D_COMPONENT_CYCLE
  };

  /**
   * Cycle the shown scalars according to the cycle type
   */
  void CycleScalars(int cycleType);

  ///@{
  /**
   * Set/Get the scalar bar actor, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(ScalarBarActor, vtkScalarBarActor);
  vtkSetSmartPointerMacro(ScalarBarActor, vtkScalarBarActor);
  ///@}

  ///@{
  /**
   * Set/Get the geometry actor
   */
  vtkGetSmartPointerMacro(GeometryActor, vtkActor);
  vtkSetSmartPointerMacro(GeometryActor, vtkActor);
  ///@}

  ///@{
  /**
   * Set/Get the point sprites actor
   */
  vtkGetSmartPointerMacro(PointSpritesActor, vtkActor);
  vtkSetSmartPointerMacro(PointSpritesActor, vtkActor);
  ///@}

  ///@{
  /**
   * Set/Get the volume prop
   */
  vtkGetSmartPointerMacro(VolumeProp, vtkVolume);
  vtkSetSmartPointerMacro(VolumeProp, vtkVolume);
  ///@}

  ///@{
  /**
   * Set/Get the polydata mapper
   */
  vtkGetSmartPointerMacro(PolyDataMapper, vtkPolyDataMapper);
  vtkSetSmartPointerMacro(PolyDataMapper, vtkPolyDataMapper);
  ///@}

  ///@{
  /**
   * Set/Get the point gaussian mapper, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(PointGaussianMapper, vtkPointGaussianMapper);
  vtkSetSmartPointerMacro(PointGaussianMapper, vtkPointGaussianMapper);
  ///@}

  ///@{
  /**
   * Set/Get the volume mapper, used for hotkey purposes
   */
  vtkGetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  vtkSetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  ///@}

  /**
   * Set the coloring attributes, call before SetColoring.
   */
  void SetColoringAttributes(vtkDataSetAttributes* pointData, vtkDataSetAttributes* cellData);

  /**
   * Set the coloring, call after SetColoringAttributes.
   * This method will try to find the corresponding array in the coloring attributes and will
   * position ArrayIndexForColoring and DataForColoring accordingly.
   */
  void SetColoring(bool useCellData, const std::string& arrayName, int component);

  ///@{
  /**
   * Get current coloring infomatiuons,
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

  static void ConfigureMapperForColoring(vtkPolyDataMapper* mapper, vtkDataArray* array,
    int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag = false);

  static void ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper, vtkVolume* volume,
    vtkDataArray* array, int component, vtkColorTransferFunction* ctf, double range[2],
    bool cellFlag = false, bool inverseOpacityFlag = false);

  static void ConfigureScalarBarActorForColoring(vtkScalarBarActor* scalarBar, vtkDataArray* array,
    int component, vtkColorTransferFunction* ctf);

  void ConfigureRangeAndCTFForColoring(vtkDataArray* array, int component);

  void UpdateScalarBarVisibility();

  void FillCheatSheetHotkeys(std::stringstream& sheet) override;

  /**
   * Look for an arrayIndex in a vtkDataSetAttributes based on an arrayName
   * Returns the arrayIndex or -1 if not found
   */
  static int FindArrayIndexForColoring(
    vtkDataSetAttributes* dataForColoring, const std::string& arrayName);

  /**
   * Switch between point data and cell data coloring
   */
  void CycleFieldForColoring();

  /**
   * Cycle the array used for coloring
   * This will iterate over the current DataForColoring
   * until it find a valid array or cycle back to non coloring
   * Set checkCurrent to true to check the current array index
   * before incrementing
   */
  void CycleArrayForColoring(bool checkCurrent = false);

  /**
   * Increment the array index or loop it back
   * When not using volume, it will loop back
   * to not coloring
   */
  void CycleArrayIndexForColoring();

  /**
   * Cycle the component in used for rendering
   * looping back to direct scalars if supported or magnitude.
   */
  void CycleComponentForColoring();

  /**
   * A method that checks if the current component is valid
   * if not, it will reset it to 0
   */
  void CheckCurrentComponentForColoring();

  std::string GenerateMetaDataDescription() override;

  /**
   * Convert a component index into a string
   */
  static std::string ComponentToString(int component);

  /**
   * Shorten a provided name with "..."
   */
  static std::string ShortName(const std::string& name, int component);

  vtkSmartPointer<vtkActor> GeometryActor;
  vtkSmartPointer<vtkActor> PointSpritesActor;
  vtkSmartPointer<vtkVolume> VolumeProp;

  vtkSmartPointer<vtkScalarBarActor> ScalarBarActor;
  bool ScalarBarActorConfigured = false;

  vtkSmartPointer<vtkPolyDataMapper> PolyDataMapper;
  bool PolyDataMapperConfigured = false;

  vtkSmartPointer<vtkPointGaussianMapper> PointGaussianMapper;
  bool PointGaussianMapperConfigured = false;

  vtkSmartPointer<vtkSmartVolumeMapper> VolumeMapper;
  bool VolumeConfigured = false;

  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
  double ColorRange[2];
  bool ColorTransferFunctionConfigured = false;

  vtkDataSetAttributes* PointDataForColoring = nullptr;
  vtkDataSetAttributes* CellDataForColoring = nullptr;
  int ArrayIndexForColoring = -1;
  int ComponentForColoring = -1;

  vtkDataSetAttributes* DataForColoring = nullptr;
  vtkDataArray* ArrayForColoring = nullptr;

  bool ScalarBarVisible = false;
  bool UsePointSprites = false;
  bool UseVolume = false;
  bool UseInverseOpacityFunction = false;

  std::vector<double> UserScalarBarRange;
  std::vector<double> Colormap;

  vtkTimeStamp ColoringTimeStamp;
  vtkMTimeType ColoringUpdateTime = 0;
};

#endif
