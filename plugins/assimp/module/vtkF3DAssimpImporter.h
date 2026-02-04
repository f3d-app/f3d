/**
 * @class   vtkF3DAssimpImporter
 * @brief   Importer using Assimp library
 *
 * This importer is based on Assimp 5 and supports all related file format.
 * The list of supported file format is available here:
 * https://github.com/assimp/assimp/blob/master/doc/Fileformats.md
 *
 * The following formats have been tested and are supported by f3d: FBX, DAE, OFF, DXF, X, 3MF

 * This importer supports reading from stream but prefer memory stream over filestream.
 * Reading from stream require to position the MemoryHint to the correct file extension.
 * No support for reading textures from streams unless embedded.
 */

#ifndef vtkF3DAssimpImporter_h
#define vtkF3DAssimpImporter_h

#include "vtkF3DImporter.h"

#include <vtkVersion.h>

#include <assimp/ProgressHandler.hpp>

#include <memory>

class vtkF3DAssimpImporter : public vtkF3DImporter
{
public:
  static vtkF3DAssimpImporter* New();
  vtkTypeMacro(vtkF3DAssimpImporter, vtkF3DImporter);

  /**
   * Update actors at the given time value.
   */
  bool UpdateAtTimeValue(double timeValue) override;

  /**
   * Get the level of animation support in this importer, which is always
   * AnimationSupportLevel::SINGLE
   */
  AnimationSupportLevel GetAnimationSupportLevel() override
  {
    return AnimationSupportLevel::SINGLE;
  }

  /**
   * Get the number of available animations.
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Return the name of the animation.
   */
  std::string GetAnimationName(vtkIdType animationIndex) override;

  ///@{
  /**
   * Enable/Disable/Get the status of specific animations
   * Only one single animation can be enabled
   * By default, no animation are enabled
   * As specified in the vtkImporter API, animationIndex
   * is expected to be 0 < GetNumberOfAnimations
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  ///@}

  /**
   * Return importer description.
   */
  std::string GetOutputsDescription() override;

  ///@{
  /**
   * Set/Get collada fixup flag.
   */
  vtkSetMacro(ColladaFixup, bool);
  vtkGetMacro(ColladaFixup, bool);
  ///@}

  /**
   * Set the hint to pass to assimp when reading from memory
   * Typically the file extension.
   */
  vtkSetMacro(MemoryHint, std::string);

  /**
   * Get temporal information for the currently enabled animation.
   * Only defines timerange and ignore provided frameRate.
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps,
    vtkDoubleArray* timeSteps) override;

  /**
   * Get the number of available cameras.
   */
  vtkIdType GetNumberOfCameras() override;

  /**
   * Get the name of a camera.
   */
  std::string GetCameraName(vtkIdType camIndex) override;

  /**
   * Enable a specific camera.
   * If a negative index is provided, no camera from the importer is used.
   */
  void SetCamera(vtkIdType camIndex) override;

protected:
  vtkF3DAssimpImporter();
  ~vtkF3DAssimpImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;
  void ImportCameras(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;

private:
  vtkF3DAssimpImporter(const vtkF3DAssimpImporter&) = delete;
  void operator=(const vtkF3DAssimpImporter&) = delete;

  bool ColladaFixup = false;
  std::string MemoryHint;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

class F3DAssimpProgressHandler : public Assimp::ProgressHandler
{
public:
  explicit F3DAssimpProgressHandler(vtkF3DAssimpImporter* parent);
  bool Update(float percentage) override;

private:
  vtkF3DAssimpImporter* Parent;
};

#endif
