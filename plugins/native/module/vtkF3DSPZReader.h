/**
 * @class   vtkF3DSPZReader
 * @brief   Reader for the SPZ file format.
 *
 * SPZ is a file format used by Niantic Labs for compressing 3D Gaussians.
 * Currently, spherical harmonics are ignored and the reader only supports constant color.
 *
 * @sa https://github.com/nianticlabs/spz/blob/main/README.md
 */

#ifndef vtkF3DSPZReader_h
#define vtkF3DSPZReader_h

#include <vtkPolyDataAlgorithm.h>

#include <memory>

class vtkF3DSPZReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DSPZReader* New();
  vtkTypeMacro(vtkF3DSPZReader, vtkPolyDataAlgorithm);

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DSPZReader();
  ~vtkF3DSPZReader() override = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSPZReader(const vtkF3DSPZReader&) = delete;
  void operator=(const vtkF3DSPZReader&) = delete;

  std::string FileName;
};

#endif
