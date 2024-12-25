/**
 * @class   vtkF3DGLTFDracoImporter
 * @brief   VTK GLTF importer with Draco support
 *
 * Subclasses the default importer to initialize our own loader.
 * @sa vtkF3DGLTFDracoDocumentLoader
 */

#ifndef vtkF3DGLTFDracoImporter_h
#define vtkF3DGLTFDracoImporter_h

#include <vtkF3DGLTFImporter.h>

class vtkF3DGLTFDracoImporter : public vtkF3DGLTFImporter
{
public:
  static vtkF3DGLTFDracoImporter* New();
  vtkTypeMacro(vtkF3DGLTFDracoImporter, vtkF3DGLTFImporter);

protected:
  vtkF3DGLTFDracoImporter() = default;
  ~vtkF3DGLTFDracoImporter() override = default;

  /**
   * Overridden to instantiate our own document loader
   */
  void InitializeLoader() override;

private:
  vtkF3DGLTFDracoImporter(const vtkF3DGLTFDracoImporter&) = delete;
  void operator=(const vtkF3DGLTFDracoImporter&) = delete;
};

#endif
