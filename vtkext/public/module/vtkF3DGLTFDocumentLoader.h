/**
 * @class   vtkF3DGLTFDocumentLoader
 * @brief   Specialized GLTF document loader with meshopt buffer decoding
 *
 * This class subclasses vtkGLTFDocumentLoader to handle meshopt metadata
 */

#ifndef vtkF3DGLTFDocumentLoader_h
#define vtkF3DGLTFDocumentLoader_h

#include "vtkextModule.h"

/// @cond
#include <vtkGLTFDocumentLoader.h>
/// @endcond

class VTKEXT_EXPORT vtkF3DGLTFDocumentLoader : public vtkGLTFDocumentLoader
{
public:
  static vtkF3DGLTFDocumentLoader* New();
  vtkTypeMacro(vtkF3DGLTFDocumentLoader, vtkGLTFDocumentLoader);

  /**
   * Overridden to add KHR_meshopt_compression and EXT_meshopt_compression support
   */
  std::vector<std::string> GetSupportedExtensions() override;

  /**
   * Overridden to handle meshopt metadata and modify the GLTF model.
   * This will take information in metadata to decode buffers and replace
   * the buffer view to point to the new decoded buffers.
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
