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

  ///@{
  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read. The stream version can move the
   * stream cursor, the filename version calls the stream version.
   *
   * This only checks that the metadata of the file can be loaded using the draco document loader.
   */
  static bool CanReadFile(vtkResourceStream* stream);
  static bool CanReadFile(const std::string& filename);
  ///@}

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
