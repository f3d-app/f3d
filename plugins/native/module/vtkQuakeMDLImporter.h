/**
 * @class   vtkQuakeMDLImporter
 * @brief   VTK Importer for Quake 1 models in binary .mdl file format
 */

#ifndef vtkQuakeMDLImporter_h
#define vtkQuakeMDLImporter_h
#include <vtkImporter.h>

class vtkQuakeMDLImporter : public vtkImporter
{
public:
  static vtkQuakeMDLImporter* New();
  vtkTypeMacro(vtkQuakeMDLImporter, vtkImporter);

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

  void UpdateTimeStep(double timeValue);

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
   * Re-render the scene after timestep
   */
  void UpdateTimeStep(double timeValue) override;

  /**
   * Enable a specific camera.
   * If a negative index is provided, no camera from the importer is used.
   */
  void SetCamera(vtkIdType camIndex) override;

protected:
  vtkQuakeMDLImporter();
  ~vtkQuakeMDLImporter() override = default;

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

  ///@{
  /**
   * Set/Get the file name.
   */
  ///@}
  std::string FileName;
  bool ColladaFixup = false;

private:
  vtkQuakeMDLImporter(const vtkQuakeMDLImporter&) = delete;
  void operator=(const vtkQuakeMDLImporter&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
