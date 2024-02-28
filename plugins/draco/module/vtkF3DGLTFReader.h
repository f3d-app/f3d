/**
 * @class   vtkF3DGLTFReader
 * @brief   VTK GLTF reader with Draco support
 *
 * Subclasses the native importer to initialize our own loader.
 * @sa vtkF3DGLTFDocumentLoader
 */

#ifndef vtkF3DGLTFReader_h
#define vtkF3DGLTFReader_h

#include <vtkGLTFReader.h>

class vtkF3DGLTFReader : public vtkGLTFReader
{
public:
  static vtkF3DGLTFReader* New();
  vtkTypeMacro(vtkF3DGLTFReader, vtkGLTFReader);

protected:
  vtkF3DGLTFReader() = default;
  ~vtkF3DGLTFReader() override = default;

  /**
   * Overridden to instantiate our own document loader
   */
  void InitializeLoader() override;

private:
  vtkF3DGLTFReader(const vtkF3DGLTFReader&) = delete;
  void operator=(const vtkF3DGLTFReader&) = delete;
};

#endif
