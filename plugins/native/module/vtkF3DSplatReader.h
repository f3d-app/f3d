/**
 * @class   vtkF3DSplatReader
 * @brief   VTK Reader for DRC files using Draco
 *
 * Reader for .splat 3D Gaussians files
 */

#ifndef vtkF3DSplatReader_h
#define vtkF3DSplatReader_h

#include <vtkPolyDataAlgorithm.h>

class vtkF3DSplatReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DSplatReader* New();
  vtkTypeMacro(vtkF3DSplatReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DSplatReader();
  ~vtkF3DSplatReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSplatReader(const vtkF3DSplatReader&) = delete;
  void operator=(const vtkF3DSplatReader&) = delete;

  std::string FileName;
};

#endif
