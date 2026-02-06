/**
 * @class   vtkF3DQuakeMDLImporter
 * @brief   VTK Importer for Quake 1 models in .mdl file format
 *
 * This reader is based on implementations of Quake 1's MDL, defined in
 * https://book.leveldesignbook.com/appendix/resources/formats/mdl It reads vertices, normals and
 * texture coordinate data from .mdl files. Supports animations.
 *
 * This reader expects a header "ident" value of either 0x4F504449 ("IDPO") or 0x54534449 ("IDST")
 * This reader expects a header "version" value of exactly 6
 */

#ifndef vtkF3DQuakeMDLImporter_h
#define vtkF3DQuakeMDLImporter_h

#include <vtkF3DImporter.h>

#include <memory>

class vtkF3DQuakeMDLImporter : public vtkF3DImporter
{
public:
  static vtkF3DQuakeMDLImporter* New();
  vtkTypeMacro(vtkF3DQuakeMDLImporter, vtkF3DImporter);

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
   */
  void EnableAnimation(vtkIdType animationIndex) override;
  void DisableAnimation(vtkIdType animationIndex) override;
  bool IsAnimationEnabled(vtkIdType animationIndex) override;
  ///@}

  /**
   * Get temporal information for the currently enabled animations.
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps,
    vtkDoubleArray* timeSteps) override;

  ///@{
  /**
   * Set/Get the skin index.
   */
  vtkSetMacro(SkinIndex, unsigned int);
  vtkGetMacro(SkinIndex, unsigned int);
  ///@}

  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a strean.
   *
   * This only checks that the file header contains the right magic ("IPDO" or "IDST") and the
   * version is 6.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DQuakeMDLImporter();
  ~vtkF3DQuakeMDLImporter() override = default;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;

private:
  vtkF3DQuakeMDLImporter(const vtkF3DQuakeMDLImporter&) = delete;
  void operator=(const vtkF3DQuakeMDLImporter&) = delete;

  struct vtkInternals;
  unsigned int SkinIndex = 0;

  std::unique_ptr<vtkInternals> Internals;
};

#endif
