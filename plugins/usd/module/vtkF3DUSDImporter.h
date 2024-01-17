/**
 * @class   vtkF3DUSDImporter
 * @brief   Importer using USD library
 *
 * Import used to load OpenUSD files.
 * It supports .usd, .usda, .usdc and .usdz formats.
 *
 * This importer should cover a large part of the features including
 * instancing and materials but there are known limitations including:
 * - Only supports preview materials
 * - Does not support UV transforms
 * - Do not support lights and cameras
 * - Skinning is done on the CPU
 * - Ignore volumes and NURBS
 *
 * @sa https://openusd.org/release/index.html
 */

#ifndef vtkF3DUSDImporter_h
#define vtkF3DUSDImporter_h

#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include <memory>

class vtkInformationStringKey;

class vtkF3DUSDImporter : public vtkImporter
{
public:
  static vtkF3DUSDImporter* New();
  vtkTypeMacro(vtkF3DUSDImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

  /**
   * Enable the animation.
   */
  void EnableAnimation(vtkIdType vtkNotUsed(animationIndex)) override
  {
    this->AnimationEnabled = true;
  }

  /**
   * Disable the animation.
   */
  void DisableAnimation(vtkIdType vtkNotUsed(animationIndex)) override
  {
    this->AnimationEnabled = false;
  }

  /**
   * Information key used to propagate the array name used as texture coordinates
   */
  static vtkInformationStringKey* TCOORDS_NAME();

protected:
  vtkF3DUSDImporter();
  ~vtkF3DUSDImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;

  bool IsAnimationEnabled(vtkIdType vtkNotUsed(animationIndex)) override
  {
    return this->AnimationEnabled;
  }

  vtkIdType GetNumberOfAnimations() override;

// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
  bool GetTemporalInformation(vtkIdType animationIndex, double frameRate, int& nbTimeSteps,
    double timeRange[2], vtkDoubleArray* timeSteps) override;
#endif

  void UpdateTimeStep(double timeStep) override;

  std::string FileName;
  bool AnimationEnabled = false;

private:
  vtkF3DUSDImporter(const vtkF3DUSDImporter&) = delete;
  void operator=(const vtkF3DUSDImporter&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
