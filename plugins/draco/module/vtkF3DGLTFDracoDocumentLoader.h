/**
 * @class   vtkF3DGLTFDracoDocumentLoader
 * @brief   Specialized GLTF document loader with Draco buffer decoding
 *
 * This class subclasses vtkGLTFDocumentLoader to handle Draco metadata
 */

#ifndef vtkF3DGLTFDracoDocumentLoader_h
#define vtkF3DGLTFDracoDocumentLoader_h

#include <vtkGLTFDocumentLoader.h>

#include "vtkF3DGLTFDocumentLoader.h"
class vtkF3DGLTFDracoDocumentLoader : public vtkF3DGLTFDocumentLoader
{
public:
  static vtkF3DGLTFDracoDocumentLoader* New();
  vtkTypeMacro(vtkF3DGLTFDracoDocumentLoader, vtkF3DGLTFDocumentLoader);

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
  vtkF3DGLTFDracoDocumentLoader() = default;
  ~vtkF3DGLTFDracoDocumentLoader() override = default;

private:
  vtkF3DGLTFDracoDocumentLoader(const vtkF3DGLTFDracoDocumentLoader&) = delete;
  void operator=(const vtkF3DGLTFDracoDocumentLoader&) = delete;
};

#endif
