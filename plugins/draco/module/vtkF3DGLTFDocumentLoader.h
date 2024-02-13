/**
 * @class   vtkF3DGLTFDocumentLoader
 * @brief   Specialized GLTF document loader with Draco buffer decoding
 *
 * This class subclasses vtkGLTFDocumentLoader to handle Draco metadata
 */

#ifndef vtkF3DGLTFDocumentLoader_h
#define vtkF3DGLTFDocumentLoader_h

#include <vtkGLTFDocumentLoader.h>

class vtkF3DGLTFDocumentLoader : public vtkGLTFDocumentLoader
{
public:
  static vtkF3DGLTFDocumentLoader* New();
  vtkTypeMacro(vtkF3DGLTFDocumentLoader, vtkGLTFDocumentLoader);

  /**
   * Overridden to add KHR_draco_mesh_compression support
   */
  std::vector<std::string> GetSupportedExtensions() override;

  /**
   * Overridden to handle Draco metadata and modify the GLTF model.
   * This will take information in metadata to decode buffers and replace
   * the buffer view in the accessors to point to the new decoded buffers.
   */
  void PrepareData() override;

protected:
  vtkF3DGLTFDocumentLoader() = default;
  ~vtkF3DGLTFDocumentLoader() override = default;

private:
  vtkF3DGLTFDocumentLoader(const vtkF3DGLTFDocumentLoader&) = delete;
  void operator=(const vtkF3DGLTFDocumentLoader&) = delete;
};

#endif
