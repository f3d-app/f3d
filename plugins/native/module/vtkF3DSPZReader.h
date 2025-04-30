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

#include <vtkAbstractPolyDataReader.h>

#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>

class vtkF3DSPZReader : public vtkAbstractPolyDataReader
{
public:
  static vtkF3DSPZReader* New();
  vtkTypeMacro(vtkF3DSPZReader, vtkPolyDataAlgorithm);

  /**
   * Specify stream to read from
   * When selecting input method, `Stream` has an higher priority than `Filename`.
   * If both are null, reader outputs nothing.
   */
  vtkSetSmartPointerMacro(Stream, vtkResourceStream);

protected:
  vtkF3DSPZReader() = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSPZReader(const vtkF3DSPZReader&) = delete;
  void operator=(const vtkF3DSPZReader&) = delete;

  vtkSmartPointer<vtkResourceStream> Stream;
};

#endif
