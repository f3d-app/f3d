/**
 * @class   vtkF3DSplatReader
 * @brief   VTK Reader for 3D Gaussians in binary .splat file format
 *
 * Reader for binary .splat files as defined in https://github.com/antimatter15/splat
 * This reader will probably evolve until there is no standard defined yet
 * An interesting discussion can be followed here:
 * https://github.com/mkkellogg/GaussianSplats3D/issues/47
 */

#ifndef vtkF3DSplatReader_h
#define vtkF3DSplatReader_h

#include <vtkPolyDataAlgorithm.h>

class vtkF3DSplatReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DSplatReader* New();
  vtkTypeMacro(vtkF3DSplatReader, vtkPolyDataAlgorithm);

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DSplatReader();
  ~vtkF3DSplatReader() override = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSplatReader(const vtkF3DSplatReader&) = delete;
  void operator=(const vtkF3DSplatReader&) = delete;

  std::string FileName;
};

#endif
