/**
 * @class   vtkF3DQuakeMDLImporter
 * @brief   VTK Importer for Quake 1 models in .mdl file format
 */

#ifndef vtkF3DQuakeMDLImporter_h
#define vtkF3DQuakeMDLImporter_h
#include "vtkF3DQuakeMDLImporterConstants.h"
#include <vtkF3DImporter.h>

class vtkF3DQuakeMDLImporter : public vtkF3DImporter
{
public:
  static vtkF3DQuakeMDLImporter* New();

  /**
   * Set the file name.
   */
  void SetFileName(std::string fileName);

  /**
   * Update actors at the given time value.
   */
  void UpdateTimeStep(double timeValue) override;

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
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  ///@}

  /**
   * Return importer description.
   */
  std::string GetOutputsDescription() override;

  /**
   * Get temporal information for the currently enabled animation.
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
   */
  void SetCamera(vtkIdType camIndex) override;

protected:
  vtkF3DQuakeMDLImporter();
  ~vtkF3DQuakeMDLImporter() override = default;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;
  void ImportCameras(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;

private:
  vtkF3DQuakeMDLImporter(const vtkF3DQuakeMDLImporter&) = delete;
  void operator=(const vtkF3DQuakeMDLImporter&) = delete;

  class vtkInternals;
  std::string FileName;

  std::unique_ptr<vtkInternals> Internals;
};

#endif
