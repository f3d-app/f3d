/**
 * @class   vtkF3DSOGReader
 * @brief   Reader for bundled PlayCanvas SOG files.
 *
 * SOG (Spatially Ordered Gaussians) stores 3D Gaussian splats as lossless WebP
 * images described by a meta.json. A bundled .sog file is a zip of those files.
 * This reader supports version 2 only. Any other meta.json version fails the whole
 * file. Higher-order spherical harmonics and blind data are ignored, so a version 2
 * file that contains them still loads, with base color only. A loose directory of
 * images is not read.
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
   * Return true if the stream looks like a bundled SOG file.
   * Checks the ZIP local-file magic (PK\x03\x04), then that meta.json is an
   * entry at the archive root. The stream position is restored. A zip without
   * that entry returns false.
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
