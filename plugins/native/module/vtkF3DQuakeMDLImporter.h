/**
 * @class   vtkF3DQuakeMDLImporter
 * @brief   VTK Importer for Quake 1 models in .mdl file format
 */

#ifndef vtkF3DQuakeMDLImporter_h
#define vtkF3DQuakeMDLImporter_h
#include <vtkImporter.h>
#include "vtkF3DQuakeMDLImporterConstants.h"

class vtkF3DQuakeMDLImporter : public vtkImporter
{
public:
  static vtkF3DQuakeMDLImporter* New();
  //vtkTypeMacro(vtkF3DQuakeMDLImporter, vtkImporter);

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
   * Only one single animation can be enabled
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
  vtkF3DQuakeMDLImporter();
  ~vtkF3DQuakeMDLImporter() override = default;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;
  void ImportCameras(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;

  // Header definition,
  struct mdl_header_t
  {
    int IDPO;
    int version;
    float scale[3];
    float translation[3];
    float boundingRadius;
    float eyePosition[3];
    int numSkins;
    int skinWidth;
    int skinHeight;
    int numVertices;
    int numTriangles;
    int numFrames;
    int syncType;
    int stateFlags;
    float size;
  };


private:
  vtkF3DQuakeMDLImporter(const vtkF3DQuakeMDLImporter&) = delete;
  void operator=(const vtkF3DQuakeMDLImporter&) = delete;

  class vtkInternals;
  std::string FileName;

  std::unique_ptr<vtkInternals> Internals;

};


#endif
