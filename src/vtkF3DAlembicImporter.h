/**
 * @class   vtkF3DAlembicImporter
 * @brief   Importer using Alembic library
 *
 * This importer is based on Alembic 1.7
 * @sa https://github.com/alembic/alembic/blob/master/README.txt
 *
 */

#ifndef vtkF3DAlembicImporter_h
#define vtkF3DAlembicImporter_h

#include <vtkImporter.h>
#include <vtkVersion.h>

class vtkF3DAlembicImporterInternalEx;

class vtkF3DAlembicImporter : public vtkImporter
{
public:
  static vtkF3DAlembicImporter* New();
  vtkTypeMacro(vtkF3DAlembicImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210118)
  //@{
  /**
   * Get/Set the file name.
   */
  vtkSetStdStringFromCharMacro(FileName);
  vtkGetCharFromStdStringMacro(FileName);
  //@}
#else
  /**
   * Get the filename
   */
  const char* GetFileName()
  {
    return this->FileName.c_str();
  }

  /**
   * Set the filename
   */
  void SetFileName(const char* fileName)
  {
    this->FileName = fileName;
    this->Modified();
  }
#endif

  /**
   * Update actors at the given timestep.
   */
  void UpdateTimeStep(double timestep) override;

  /**
   * Get the number of available animations.
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Return the name of the animation.
   */
  std::string GetAnimationName(vtkIdType animationIndex) override;

  //@{
  /**
   * Enable/Disable/Get the status of specific animations
   * Only one single animation can be enabled
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  //@}

  /**
   * Return importer description.
   */
  std::string GetOutputsDescription() override;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200912)
  /**
   * Get temporal informations for the currently enabled animations.
   * the three returned arguments can be defined, or not.
   * Return true in case of success, false otherwise.
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
#endif

protected:
  vtkF3DAlembicImporter();
  ~vtkF3DAlembicImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;
  void ImportCameras(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;

  std::string FileName;

private:
  vtkF3DAlembicImporter(const vtkF3DAlembicImporter&) = delete;
  void operator=(const vtkF3DAlembicImporter&) = delete;

  vtkF3DAlembicImporterInternalEx* InternalsEx = nullptr;
};

#endif
