/**
 * @class   vtkF3DAlembicImporter
 * @brief   Importer using Alembic library
 *
 * This importer is based on Alembic 1.7
 * Currently, only polygonal points positions are retrieved
 * to build polygonal geometries. Vertex normals and texture
 * coordinates are not supported yet.
 *
 * @sa https://github.com/alembic/alembic/blob/master/README.txt
 *
 */

#ifndef vtkF3DAlembicImporter_h
#define vtkF3DAlembicImporter_h

#include <memory>
#include <vtkImporter.h>
#include <vtkVersion.h>
#include <vtkNew.h>

class vtkF3DAlembicImporterInternal;

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
  const char* GetFileName() { return this->FileName.c_str(); }

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
   *
   * \todo Cache/animation support not implemented yet
   */
  void UpdateTimeStep(double timestep) override;

  /**
   * Get the number of available animations.
   *
   * \todo Cache/animation support not implemented yet
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Return the name of the animation.
   *
   * \todo Cache/animation support not implemented yet
   */
  std::string GetAnimationName(vtkIdType animationIndex) override;

  //@{
  /**
   * Enable/Disable/Get the status of specific animations
   *
   * \todo Cache/animation support not implemented yet
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
   *
   * \todo Cache/animation support not implemented yet
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;

#endif

protected:
  vtkF3DAlembicImporter() = default;
  ~vtkF3DAlembicImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;

  std::string FileName;

private:
  vtkF3DAlembicImporter(const vtkF3DAlembicImporter&) = delete;
  void operator=(const vtkF3DAlembicImporter&) = delete;

  vtkNew<vtkF3DAlembicImporterInternal> Internals;
};

#endif
