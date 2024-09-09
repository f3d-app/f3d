/**
 * @class   vtkF3DAssimpImporter
 * @brief   Importer using Assimp library
 *
 * This importer is based on Assimp 5 and supports all related file format.
 * The list of supported file format is available here:
 * https://github.com/assimp/assimp/blob/master/doc/Fileformats.md
 *
 * The following formats have been tested and are supported by f3d: FBX, DAE, OFF, DXF, X, 3MF
 */

#ifndef vtkF3DAssimpImporter_h
#define vtkF3DAssimpImporter_h

#include "vtkF3DImporter.h"

#include <vtkVersion.h>

#include <memory>

class vtkF3DAssimpImporter : public vtkF3DImporter
{
public:
  static vtkF3DAssimpImporter* New();
  vtkTypeMacro(vtkF3DAssimpImporter, vtkF3DImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Set/Get the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

  /**
   * Update actors at the given time value.
   */
  bool UpdateAtTimeValue(double timeValue) override;

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
   * Get temporal information for the currently enabled animation.
   * Only defines timerange and ignore provided frameRate.
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;

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

  std::string FileName;
  bool ColladaFixup = false;

private:
  vtkF3DAssimpImporter(const vtkF3DAssimpImporter&) = delete;
  void operator=(const vtkF3DAssimpImporter&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
