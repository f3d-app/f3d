/**
 * @class   vtkF3D3DSImporter
 * @brief   VTK importer for 3D Studio files with PBR materials
 */

#ifndef vtkF3D3DSImporter_h
#define vtkF3D3DSImporter_h

#include <vtk3DSImporter.h>

#include <memory>

class vtkF3D3DSImporter : public vtk3DSImporter
{
public:
  static vtkF3D3DSImporter* New();
  vtkTypeMacro(vtkF3D3DSImporter, vtk3DSImporter);

protected:
  vtkF3D3DSImporter();
  ~vtkF3D3DSImporter() override = default;

  void ImportProperties(vtkRenderer* renderer) override;

private:
  vtkF3D3DSImporter(const vtkF3D3DSImporter&) = delete;
  void operator=(const vtkF3D3DSImporter&) = delete;
};

#endif
