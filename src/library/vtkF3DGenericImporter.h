/**
 * @class   vtkF3DGenericImporter
 * @brief   create a scene from the meta reader
 */

#ifndef vtkF3DGenericImporter_h
#define vtkF3DGenericImporter_h

#include "vtkF3DMetaReader.h"
#include "vtkF3DPostProcessFilter.h"

#include <vtkImporter.h>
#include <vtkVersion.h>

namespace f3d
{
class options;
}
class vtkActor;
class vtkVolume;
class vtkPolyDataMapper;
class vtkMultiBlockDataSet;
class vtkPointGaussianMapper;
class vtkPolyDataMapper;
class vtkSmartVolumeMapper;
class vtkScalarBarActor;
class vtkTexture;

class vtkF3DGenericImporter : public vtkImporter
{
public:
  static vtkF3DGenericImporter* New();

  vtkTypeMacro(vtkF3DGenericImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Specify the name of the file to read.
   */
  void SetFileName(const char* arg);

  /**
   * Set options.
   */
  void SetOptions(const f3d::options* options);

  /**
   * Check if the file can be read.
   */
  bool CanReadFile();

  /**
   * Get a string describing the outputs
   */
  std::string GetOutputsDescription() override;

  //@{
  /**
   * Static methods to recover information about data
   */
  static std::string GetMultiBlockDescription(vtkMultiBlockDataSet* mb, vtkIndent indent);
  static std::string GetDataObjectDescription(vtkDataObject* object);
  //@}

  //@{
  /**
   * Access to specific actors
   */
  vtkGetSmartPointerMacro(ScalarBarActor, vtkScalarBarActor);
  vtkGetSmartPointerMacro(GeometryActor, vtkActor);
  vtkGetSmartPointerMacro(PointSpritesActor, vtkActor);
  vtkGetSmartPointerMacro(VolumeProp, vtkVolume);
  //@}

  //@{
  /**
   * Access to specific mappers
   */
  vtkGetSmartPointerMacro(PolyDataMapper, vtkPolyDataMapper);
  vtkGetSmartPointerMacro(PointGaussianMapper, vtkPointGaussianMapper);
  vtkGetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  //@}

  //@{
  /**
   * Access to specific flags
   */
  vtkGetMacro(ArrayIndexForColoring, int);
  vtkGetObjectMacro(PointDataForColoring, vtkDataSetAttributes);
  vtkGetObjectMacro(CellDataForColoring, vtkDataSetAttributes);
  //@}

  void UpdateTimeStep(double timestep) override;

  /**
   * Get the number of available animations.
   * Returns 1 if an animation is available or
   * 0 if not.
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Return a dummy name of the first animation if any, empty string otherwise.
   */
  std::string GetAnimationName(vtkIdType animationIndex) override;

  //@{
  /**
   * Enable/Disable/Get the status of specific animations
   * Only the first animation can be enabled
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  //@}

  /**
   * Get temporal informations for the currently enabled animations.
   * Framerate is ignored in this implementation.
   * the three return arguments are defined in this implementation.
   */
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;
#else
  bool GetTemporalInformation(vtkIdType animationIndex, int& nbTimeSteps, double timeRange[2],
    vtkDoubleArray* timeSteps) override;
#endif

protected:
  vtkF3DGenericImporter() = default;
  ~vtkF3DGenericImporter() override = default;

  void ImportActors(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;
  void ImportProperties(vtkRenderer*) override;

  vtkSmartPointer<vtkTexture> GetTexture(const std::string& fileName, bool isSRGB = false);

  void UpdateTemporalInformation();

  vtkNew<vtkF3DMetaReader> Reader;

  const f3d::options* Options = nullptr;
  vtkNew<vtkScalarBarActor> ScalarBarActor;
  vtkNew<vtkActor> GeometryActor;
  vtkNew<vtkActor> PointSpritesActor;
  vtkNew<vtkVolume> VolumeProp;
  vtkNew<vtkPolyDataMapper> PolyDataMapper;
  vtkNew<vtkPointGaussianMapper> PointGaussianMapper;
  vtkNew<vtkSmartVolumeMapper> VolumeMapper;
  std::string OutputDescription;

  vtkDataSetAttributes* PointDataForColoring = nullptr;
  vtkDataSetAttributes* CellDataForColoring = nullptr;
  int ArrayIndexForColoring = 0;

  bool AnimationEnabled = false;
  bool TemporalInformationUpdated = false;
  int NbTimeSteps = -1;
  double* TimeSteps = nullptr;
  double* TimeRange = nullptr;
  vtkNew<vtkF3DPostProcessFilter> PostPro;

private:
  vtkF3DGenericImporter(const vtkF3DGenericImporter&) = delete;
  void operator=(const vtkF3DGenericImporter&) = delete;
};

#endif
