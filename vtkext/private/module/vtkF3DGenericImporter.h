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
class vtkPartitionedDataSet;
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
  bool GetTemporalInformation(vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps,
    vtkDoubleArray* timeSteps) override;

  ///@{
  /**
   * Direct access to generic importer specific datasets.
   * The no-argument versions return data for the first block.
   * The indexed versions return data for the specified actor/block index.
   */
  vtkPolyData* GetImportedPoints();
  vtkPolyData* GetImportedPoints(vtkIdType actorIndex);
  vtkImageData* GetImportedImage();
  vtkImageData* GetImportedImage(vtkIdType actorIndex);
  ///@}

  /**
   * Get the name of a block by its actor index.
   * Returns an empty string if the index is invalid.
   */
  std::string GetBlockName(vtkIdType actorIndex);

  /**
   * Get the number of blocks/actors created by this importer.
   */
  vtkIdType GetNumberOfBlocks();

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

  /**
   * Create an actor for a single dataset block
   */
  void CreateActorForBlock(vtkDataSet* block, vtkRenderer* ren, const std::string& blockName = "");

  /**
   * Import blocks from a vtkMultiBlockDataSet with proper name extraction
   */
  void ImportMultiBlock(
    vtkMultiBlockDataSet* mb, vtkRenderer* ren, const std::string& parentName = "");

  /**
   * Import blocks from a vtkPartitionedDataSetCollection with proper name extraction
   */
  void ImportPartitionedDataSetCollection(vtkPartitionedDataSetCollection* pdc, vtkRenderer* ren);

  /**
   * Import blocks from a vtkPartitionedDataSet with proper name extraction
   */
  void ImportPartitionedDataSet(
    vtkPartitionedDataSet* pds, vtkRenderer* ren, const std::string& pdsName = "");

  struct Internals;
  std::unique_ptr<Internals> Pimpl;
};

#endif
