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

#include "vtkF3DImporter.h"

#include <vtkNew.h>
#include <vtkVersion.h>

#include <memory>
#include <string>

class vtkInformationStringKey;
class vtkResourceStream;

class vtkF3DUSDImporter : public vtkF3DImporter
{
public:
  static vtkF3DUSDImporter* New();
  vtkTypeMacro(vtkF3DUSDImporter, vtkF3DImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Get the level of animation support in this importer, which is always
   * AnimationSupportLevel::UNIQUE
   */
  AnimationSupportLevel GetAnimationSupportLevel() override
  {
    return AnimationSupportLevel::UNIQUE;
  }

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
   * Return if the animation is enabled
   */
  bool IsAnimationEnabled(vtkIdType vtkNotUsed(animationIndex)) override
  {
    return this->AnimationEnabled;
  }

  /**
   * Return 1 if an animation is available, 0 otherwise
   */
  vtkIdType GetNumberOfAnimations() override;

  /**
   * Information key used to propagate the array name used as texture coordinates
   */
  static vtkInformationStringKey* TCOORDS_NAME();

  ///@{
  /**
   * Check whether the given stream contains a recognizable USD format.
   * The two-argument overload also sets `hint` to the detected file extension
   * ("usda", "usdc", or "usdz") which is required when reading from a stream.
   */
  static bool CanReadFile(vtkResourceStream* stream);
  static bool CanReadFile(vtkResourceStream* stream, std::string& hint);
  ///@}

  /**
   * Recover animation timeRange, all other args are ignored
   */
  bool GetTemporalInformation(vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps,
    vtkDoubleArray* timeSteps) override;

  /**
   * Update importer at provided timeValue
   */
  bool UpdateAtTimeValue(double timeValue) override;

  /**
   * Set additional resources path to find plugInfo.json files
   * It's usually not needed except if the files location is not relative to the binaries,
   * which can be the case on sandboxed environment like Android.
   */
  void SetResourcesPath(const std::string& path);

protected:
  vtkF3DUSDImporter();
  ~vtkF3DUSDImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;

private:
  vtkF3DUSDImporter(const vtkF3DUSDImporter&) = delete;
  void operator=(const vtkF3DUSDImporter&) = delete;

  bool AnimationEnabled = false;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
