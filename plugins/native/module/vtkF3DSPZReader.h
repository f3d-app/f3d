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

class vtkResourceStream;

class vtkF3DSPZReader : public vtkAbstractPolyDataReader
{
public:
  static vtkF3DSPZReader* New();
  vtkTypeMacro(vtkF3DSPZReader, vtkPolyDataAlgorithm);

  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a stream.
   *
   * This only checks this is a gunzip file using the 1F8B magic.
   * This is ok as SPZ is the only gunzip based format supported by F3D.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DSPZReader() = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSPZReader(const vtkF3DSPZReader&) = delete;
  void operator=(const vtkF3DSPZReader&) = delete;
};

#endif
