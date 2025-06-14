/**
 * @class   vtkF3DGenericImporter
 * @brief   create a scene for any vtkAlgorithm
 */

#ifndef vtkF3DGenericImporter_h
#define vtkF3DGenericImporter_h

#include "vtkF3DImporter.h"

#include <memory>

class vtkAlgorithm;
class vtkDataObject;
class vtkImageData;
class vtkMultiBlockDataSet;
class vtkPartitionedDataSetCollection;
class vtkPolyData;
class vtkF3DGenericImporter : public vtkF3DImporter
{
public:
  static vtkF3DGenericImporter* New();

  vtkTypeMacro(vtkF3DGenericImporter, vtkF3DImporter);

  /**
   * Set the internal reader to recover actors and data from
   */
  void SetInternalReader(vtkAlgorithm* reader);

  /**
   * Get a string describing the outputs
   */
  std::string GetOutputsDescription() override;

  ///@{
  /**
   * Static methods to recover information about data
   */
  static std::string GetMultiBlockDescription(vtkMultiBlockDataSet* mb, vtkIndent indent);
  static std::string GetPartitionedDataSetCollectionDescription(
    vtkPartitionedDataSetCollection* pdc, vtkIndent indent);
  static std::string GetDataObjectDescription(vtkDataObject* object);
  ///@}

  /**
   * Update internal reader on the specified timestep
   */
  bool UpdateAtTimeValue(double timeValue) override;

  /**
   * Get the level of animation support in this importer, which is always
   * AnimationSupportLevel::UNIQUE
   */
  AnimationSupportLevel GetAnimationSupportLevel() override
  {
    return AnimationSupportLevel::UNIQUE;
  }

  /**
   * Get the number of available animations.
   * Returns 1 if an animation is available or
   * 0 if not.
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Return "default" for the first animation if any, empty string otherwise.
   */
  std::string GetAnimationName(vtkIdType animationIndex) override;

  ///@{
  /**
   * Enable/Disable/Get the status of specific animations
   * Only the first animation can be enabled
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  ///@}

  /**
   * Get temporal information for the currently enabled animations.
   * Framerate is ignored in this implementation.
   * Only timerange is defined in this implementation.
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;

  ///@{
  /**
   * Direct access to generic importer specific datasets
   */
  vtkPolyData* GetImportedPoints();
  vtkImageData* GetImportedImage();
  ///@}

protected:
  vtkF3DGenericImporter();
  ~vtkF3DGenericImporter() override = default;

  /*
   * Import surface from the internal reader output as actors
   */
  void ImportActors(vtkRenderer*) override;

  /**
   * Update temporal information according to currently added readers
   */
  void UpdateTemporalInformation();

  /**
   * Update output descriptions according to current outputs
   */
  void UpdateOutputDescriptions();

private:
  vtkF3DGenericImporter(const vtkF3DGenericImporter&) = delete;
  void operator=(const vtkF3DGenericImporter&) = delete;

  struct Internals;
  std::unique_ptr<Internals> Pimpl;
};

#endif
