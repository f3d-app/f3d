/**
 * @class   vtkF3DGLTFImporter
 * @brief   VTK GLTF importer with Draco support
 *
 * Subclasses the native importer to initialize our own loader.
 * @sa vtkF3DGLTFDocumentLoader
 */

#ifndef vtkF3DGLTFImporter_h
#define vtkF3DGLTFImporter_h

#include <vtkGLTFImporter.h>

class vtkF3DGLTFImporter : public vtkGLTFImporter
{
public:
  static vtkF3DGLTFImporter* New();
  vtkTypeMacro(vtkF3DGLTFImporter, vtkGLTFImporter);

protected:
  vtkF3DGLTFImporter() = default;
  ~vtkF3DGLTFImporter() override = default;

  /**
   * Overridden to instantiate our own document loader
   */
  void InitializeLoader() override;

private:
  vtkF3DGLTFImporter(const vtkF3DGLTFImporter&) = delete;
  void operator=(const vtkF3DGLTFImporter&) = delete;
};

#endif
