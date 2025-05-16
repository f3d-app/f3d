/**
 * @class   vtkF3DPLYReader
 * @brief   VTK Reader for 3D Gaussians in binary .ply file format
 *
 * Reader for "classic" INRIA .ply files as defined in https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/
 * Supports 3rd degree spherical harmonics.
 */

#ifndef vtkF3DPLYReader_h
#define vtkF3DPLYReader_h

#include <vtkPLYReader.h>

class vtkF3DPLYReader : public vtkPLYReader
{
public:
  static vtkF3DPLYReader* New();
  vtkTypeMacro(vtkF3DPLYReader, vtkPLYReader);

protected:
  vtkF3DPLYReader() = default;
  ~vtkF3DPLYReader() override = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DPLYReader(const vtkF3DPLYReader&) = delete;
  void operator=(const vtkF3DPLYReader&) = delete;
};

#endif
