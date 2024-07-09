/**
 * @class   vtkF3DGenericImporter
 * @brief   create a scene from the meta reader
 */

#ifndef vtkF3DGenericImporter_h
#define vtkF3DGenericImporter_h

#include "vtkF3DImporter.h"
#include "vtkF3DPostProcessFilter.h"

#include <vtkBoundingBox.h>
#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

#include <array>
#include <limits>
#include <memory>
#include <set>
#include <vector>

class vtkActor;
class vtkVolume;
class vtkMultiBlockDataSet;
class vtkPartitionedDataSetCollection;
class vtkPointGaussianMapper;
class vtkPolyDataMapper;
class vtkSmartVolumeMapper;
class vtkTexture;

class vtkF3DGenericImporter : public vtkF3DImporter
{
public:
  static vtkF3DGenericImporter* New();

  vtkTypeMacro(vtkF3DGenericImporter, vtkF3DImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Add an internal reader to generate actors from
   */
  void AddInternalReader(const std::string& name, vtkAlgorithm* reader);

  /**
   * Remove all internal readers
   */
  void RemoveInternalReaders();

  /**
   * Check if the file can be read.
   */
  bool CanReadFile();

  /**
   * Get a string describing the outputs
   */
  std::string GetOutputsDescription() override;
  std::string GetMetaDataDescription(); // TODO add to vtkImporter in VTK ?

  ///@{
  /**
   * Static methods to recover information about data
   */
  static std::string GetMultiBlockDescription(vtkMultiBlockDataSet* mb, vtkIndent indent);
  static std::string GetPartitionedDataSetCollectionDescription(
    vtkPartitionedDataSetCollection* pdc, vtkIndent indent);
  static std::string GetDataObjectDescription(vtkDataObject* object);
  static std::string GetMetaDataDescription(vtkDataObject* object);
  ///@}

  ///@{
  /**
   * Access to actors vectors. They all have the same size, which correspond to the number
   * of added internal readers.
   */
  std::vector<std::pair<vtkActor*, vtkPolyDataMapper*>> GetGeometryActorsAndMappers();
  std::vector<std::pair<vtkActor*, vtkPointGaussianMapper*>> GetPointSpritesActorsAndMappers();
  std::vector<std::pair<vtkVolume*, vtkSmartVolumeMapper*>> GetVolumePropsAndMappers();
  ///@}

  /**
   * A struct containing information about possible coloring
   */
  struct ColoringInfo
  {
    std::string Name;
    int MaximumNumberOfComponents = 0;
    std::vector<std::string> ComponentNames;
    std::vector<std::array<double, 2>> ComponentRanges;
    std::array<double, 2> MagnitudeRange = { std::numeric_limits<float>::max(),
      std::numeric_limits<float>::min() };
    std::vector<vtkDataArray*> Arrays;
  };

  /**
   * Recover information about coloring by index
   * Should be called after actors have been imported
   */
  bool GetInfoForColoring(bool useCellData, int index, ColoringInfo& info);

  /**
   * Get the maximum index possible for coloring
   * Should be called after actors have been imported
   */
  int GetNumberOfIndexesForColoring(bool useCellData);

  /**
   * Find an index for coloring corresponding to provided arrayName if available
   * Should be called after actors have been imported
   */
  int FindIndexForColoring(bool useCellData, const std::string& arrayName);

  /**
   * Get the bounding box of all geometry actors
   * Should be called after actors have been imported
   */
  const vtkBoundingBox& GetGeometryBoundingBox();

  /**
   * Update readers and all pipelines on the specified timestep
   */
  bool UpdateAtTimeValue(double timeValue) override;

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
// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;
#else
  bool GetTemporalInformation(vtkIdType animationIndex, int& nbTimeSteps, double timeRange[2],
    vtkDoubleArray* timeSteps) override;
#endif

protected:
  vtkF3DGenericImporter();
  ~vtkF3DGenericImporter() override = default;

  /* Standard ImportActors
   * None of the actors are shown by default
   */
  void ImportActors(vtkRenderer*) override;

  /**
   * Update temporal information according to currently added readers
   */
  void UpdateTemporalInformation();

  /**
   * Update coloring information vectors according to
   * currently added vectors
   */
  void UpdateColoringVectors(bool useCellData);

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
