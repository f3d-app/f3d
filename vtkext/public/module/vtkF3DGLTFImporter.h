/**
 * @class   vtkF3DGLTFImporter
 * @brief   VTK GLTF importer customization
 *
 * Subclasses the native importer to modify the armature shader.
 */

#ifndef vtkF3DGLTFImporter_h
#define vtkF3DGLTFImporter_h

#include "vtkextModule.h"

#include <vtkGLTFImporter.h>
#include <vtkVersion.h>

class vtkInformationIntegerKey;

class VTKEXT_EXPORT vtkF3DGLTFImporter : public vtkGLTFImporter
{
public:
  static vtkF3DGLTFImporter* New();
  vtkTypeMacro(vtkF3DGLTFImporter, vtkGLTFImporter);

protected:
  vtkF3DGLTFImporter();
  ~vtkF3DGLTFImporter() override = default;

  // need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11774
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20241219)
  /**
   * This method is reimplemented to add information to the actor in order
   * to properly draw armatures on top.
   */
  void ApplyArmatureProperties(vtkActor* actor) override;
#endif

private:
  vtkF3DGLTFImporter(const vtkF3DGLTFImporter&) = delete;
  void operator=(const vtkF3DGLTFImporter&) = delete;
};

#endif
