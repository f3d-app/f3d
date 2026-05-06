/**
 * @class   vtkF3DOpen3SDCMImporter
 * @brief   Importer for 3Shape DCM files using Open3SDCM library
 *
 * This importer uses the Open3SDCM library to parse 3Shape DCM files
 * (Dental CAD Model format) and convert them to VTK actors with geometry,
 * texture coordinates, and embedded textures.
 * 
 * 3Shape DCM files are ZIP archives containing HPS (Himsa Packed Scan) XML
 * with base64-encoded binary vertex and facet data.
 * 
 * Supported schemas: CA, CB, CC (unencrypted), CE (encrypted with Blowfish)
 * 
 * @warning This importer requires the Open3SDCM library and its dependencies
 * (Poco, OpenSSL) to be available at build time.
 */

#ifndef vtkF3DOpen3SDCMImporter_h
#define vtkF3DOpen3SDCMImporter_h

#include "vtkF3DImporter.h"

#include <memory>
#include <string>

class vtkResourceStream;

class vtkF3DOpen3SDCMImporter : public vtkF3DImporter
{
public:
  /**
   * Instantiate this class.
   */
  static vtkF3DOpen3SDCMImporter* New();

  /**
   * Print the class name and version.
   */
  vtkTypeMacro(vtkF3DOpen3SDCMImporter, vtkF3DImporter);

  /**
   * Check if the importer can read the given file.
   * Returns true if the file is a valid 3Shape DCM file.
   */
  static bool CanReadFile(const std::string& filename);

  /**
   * Check if the importer can read the given stream.
   * Currently always returns false as stream reading is not supported.
   */
  static bool CanReadStream(vtkResourceStream* stream);

  /**
   * Get the level of animation support in this importer, which is always
   * AnimationSupportLevel::NONE
   */
  AnimationSupportLevel GetAnimationSupportLevel() override
  {
    return AnimationSupportLevel::NONE;
  }

  /**
   * Get the number of available animations.
   */
  vtkIdType GetNumberOfAnimations() override { return 0; }

  /**
   * Return the name of the animation.
   */
  std::string GetAnimationName(vtkIdType vtkNotUsed(animationIndex)) override { return ""; }

  /**
   * Enable a specific animation.
   */
  void EnableAnimation(vtkIdType vtkNotUsed(animationIndex)) override {}

  /**
   * Disable a specific animation.
   */
  void DisableAnimation(vtkIdType vtkNotUsed(animationIndex)) override {}

  /**
   * Return true if a specific animation is enabled.
   */
  bool IsAnimationEnabled(vtkIdType vtkNotUsed(animationIndex)) override { return false; }

  /**
   * Get temporal information for the currently enabled animation.
   */
  bool GetTemporalInformation(vtkIdType vtkNotUsed(animationIndex), double vtkNotUsed(timeRange)[2], 
    int& vtkNotUsed(nbTimeSteps), vtkDoubleArray* vtkNotUsed(timeSteps)) override
  {
    return false;
  }

  /**
   * Get the number of available cameras.
   */
  vtkIdType GetNumberOfCameras() override { return 0; }

  /**
   * Get the name of a camera.
   */
  std::string GetCameraName(vtkIdType vtkNotUsed(camIndex)) override { return ""; }

  /**
   * Enable a specific camera.
   */
  void SetCamera(vtkIdType vtkNotUsed(camIndex)) override {}

  /**
   * Get importer description.
   */
  std::string GetOutputsDescription() override;

protected:
  vtkF3DOpen3SDCMImporter();
  ~vtkF3DOpen3SDCMImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer* renderer) override;
  void ImportCameras(vtkRenderer* vtkNotUsed(renderer)) override {}
  void ImportLights(vtkRenderer* vtkNotUsed(renderer)) override {}

private:
  vtkF3DOpen3SDCMImporter(const vtkF3DOpen3SDCMImporter&) = delete;
  void operator=(const vtkF3DOpen3SDCMImporter&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif // vtkF3DOpen3SDCMImporter_h
