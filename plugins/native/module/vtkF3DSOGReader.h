/**
 * @class   vtkF3DSOGReader
 * @brief   Reader for the SOG file format.
 *
 * SOG is the PlayCanvas format for 3D Gaussians. A .sog file is a zip of
 * lossless WebP images plus meta.json.
 * Only version 2 is supported. Any other version fails the file.
 * Higher-order spherical harmonics and blind data are ignored, so a version 2
 * file that has them still loads, with base color only.
 * A loose folder of images is not read.
 *
 * @sa https://developer.playcanvas.com/user-manual/gaussian-splatting/formats/sog/
 */

#ifndef vtkF3DSOGReader_h
#define vtkF3DSOGReader_h

#include <vtkAbstractPolyDataReader.h>

class vtkResourceStream;

class vtkF3DSOGReader : public vtkAbstractPolyDataReader
{
public:
  static vtkF3DSOGReader* New();
  vtkTypeMacro(vtkF3DSOGReader, vtkPolyDataAlgorithm);

  /**
   * Return true if, after a quick check of the file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a stream.
   *
   * Checks the ZIP magic (PK\x03\x04), then that meta.json is at the archive root.
   * Puts the stream back afterwards. A zip without that entry does not pass.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DSOGReader() = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DSOGReader(const vtkF3DSOGReader&) = delete;
  void operator=(const vtkF3DSOGReader&) = delete;
};

#endif
