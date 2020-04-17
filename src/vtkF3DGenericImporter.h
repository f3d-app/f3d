/**
 * @class   vtkF3DGenericImporter
 * @brief   create a scene from the meta reader
 */

#ifndef vtkF3DGenericImporter_h
#define vtkF3DGenericImporter_h

#include "Config.h"

#include "vtkF3DMetaReader.h"

#include <vtkImporter.h>

class vtkActor;
class vtkVolume;
class vtkPolyDataMapper;
class vtkMultiBlockDataSet;
class vtkPointGaussianMapper;
class vtkPolyDataMapper;
class vtkSmartVolumeMapper;
class vtkScalarBarActor;
class vtkColorTransferFunction;
class vtkTexture;
struct F3DOptions;

class vtkF3DGenericImporter : public vtkImporter
{
public:
  static vtkF3DGenericImporter* New();

  vtkTypeMacro(vtkF3DGenericImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Specify the name of the file to read.
   */
  void SetFileName(const char* arg);

  /**
   * Set options.
   */
  void SetOptions(const F3DOptions& options);

  /**
   * Check if the file can be read.
   */
  bool CanReadFile();

  /**
   * Get a string describing the outputs
   */
  std::string GetOutputsDescription() override;

  //@{
  /**
   * Static methods to recover information about data
   */
  static std::string GetMultiBlockDescription(vtkMultiBlockDataSet* mb, vtkIndent indent);
  static std::string GetDataObjectDescription(vtkDataObject* object);
  //@}

  //@{
  /**
   * Access to specific actors
   */
  vtkGetSmartPointerMacro(ScalarBarActor, vtkScalarBarActor);
  vtkGetSmartPointerMacro(GeometryActor, vtkActor);
  vtkGetSmartPointerMacro(PointSpritesActor, vtkActor);
  vtkGetSmartPointerMacro(VolumeProp, vtkVolume);
  //@}

  //@{
  /**
   * Access to specific mappers
   */
  vtkGetSmartPointerMacro(PolyDataMapper, vtkPolyDataMapper);
  vtkGetSmartPointerMacro(PointGaussianMapper, vtkPointGaussianMapper);
  vtkGetSmartPointerMacro(VolumeMapper, vtkSmartVolumeMapper);
  //@}

protected:
  vtkF3DGenericImporter() = default;
  ~vtkF3DGenericImporter() override = default;

  void ImportActors(vtkRenderer*) override;
  void ImportLights(vtkRenderer*) override;
  void ImportProperties(vtkRenderer*) override;

  vtkSmartPointer<vtkTexture> GetTexture(const std::string &fileName, bool isSRGB = false);

  void ConfigureMapperForColoring(
    vtkPolyDataMapper* mapper, vtkDataArray* array, vtkColorTransferFunction* ctf, double range[2]);

  void ConfigureVolumeForColoring(
    vtkVolume* volume, vtkDataArray* array, vtkColorTransferFunction* ctf, double range[2]);

  vtkNew<vtkF3DMetaReader> Reader;

  const F3DOptions* Options = nullptr;
  vtkNew<vtkScalarBarActor> ScalarBarActor;
  vtkNew<vtkActor> GeometryActor;
  vtkNew<vtkActor> PointSpritesActor;
  vtkNew<vtkVolume> VolumeProp;
  vtkNew<vtkPolyDataMapper> PolyDataMapper;
  vtkNew<vtkPointGaussianMapper> PointGaussianMapper;
  vtkNew<vtkSmartVolumeMapper> VolumeMapper;
  std::string OutputDescription;

private:
  vtkF3DGenericImporter(const vtkF3DGenericImporter&) = delete;
  void operator=(const vtkF3DGenericImporter&) = delete;
};

#endif
