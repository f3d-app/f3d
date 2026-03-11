/**
 * @class   vtkF3DSPZReader
 * @brief   Reader for the SPZ file format.
 *
 * SPZ is a file format used by Niantic Labs for compressing 3D Gaussians.
 * It supports up to 3 spherical harmonics degrees, and many attributes are quantized.
 * Moreover, the file is gzipped.
 *
 * @sa https://github.com/nianticlabs/spz/blob/main/README.md
 */

#ifndef vtkF3DSPZReader_h
#define vtkF3DSPZReader_h

#include <vtkAbstractPolyDataReader.h>

class vtkF3DSPZReader : public vtkAbstractPolyDataReader
{
public:
  static vtkF3DSPZReader* New();
  vtkTypeMacro(vtkF3DSPZReader, vtkPolyDataAlgorithm);

protected:
  vtkF3DSPZReader() = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSPZReader(const vtkF3DSPZReader&) = delete;
  void operator=(const vtkF3DSPZReader&) = delete;
};

#endif
