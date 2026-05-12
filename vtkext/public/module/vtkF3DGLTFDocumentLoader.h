/**
 * @class   vtkF3DGLTFDocumentLoader
 * @brief   Specialized GLTF document loader with EXT_texture_webp support
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
   * Overridden to add EXT_texture_webp support
   */
  std::vector<std::string> GetSupportedExtensions() override;

protected:
  vtkF3DGLTFDocumentLoader() = default;
  ~vtkF3DGLTFDocumentLoader() override = default;

private:
  vtkF3DGLTFDocumentLoader(const vtkF3DGLTFDocumentLoader&) = delete;
  void operator=(const vtkF3DGLTFDocumentLoader&) = delete;
};

#endif