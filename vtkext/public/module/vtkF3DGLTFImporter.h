/**
 * @class   vtkF3DGLTFImporter
 * @brief   VTK GLTF importer customization
 *
 * Subclasses the native importer to modify the armature shader.
 */

#ifndef vtkF3DGLTFImporter_h
#define vtkF3DGLTFImporter_h

#include "vtkextModule.h"

/// @cond
#include <vtkGLTFImporter.h>
#include <vtkVersion.h>
/// @endcond

class vtkInformationIntegerKey;

class VTKEXT_EXPORT vtkF3DGLTFImporter : public vtkGLTFImporter
{
public:
  static vtkF3DGLTFImporter* New();
  vtkTypeMacro(vtkF3DGLTFImporter, vtkGLTFImporter);

  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read. The stream version can move the
   * stream cursor, the filename version calls the stream version.
   *
   * This only checks that the metadata of the file can be loaded using the f3d document loader.
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DGLTFImporter();
  ~vtkF3DGLTFImporter() override = default;

  /**
   * Overridden to instantiate our own document loader
   */
  void InitializeLoader() override;

  /**
   * Overridden to process SH arrays
   */
  int ImportBegin() override;

  /**
   * This method is reimplemented to add information to the actor in order
   * to properly draw armatures on top.
   */
  void ApplyArmatureProperties(vtkActor* actor) override;

  // need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
  // which is backported in 9.6.2 in https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13185
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 2)
  /**
   * This method is reimplemented to add a workaround needed before the fix linked above.
   */
  void ImportActors(vtkRenderer* renderer) override;
#endif

private:
  vtkF3DGLTFImporter(const vtkF3DGLTFImporter&) = delete;
  void operator=(const vtkF3DGLTFImporter&) = delete;
};

#endif
