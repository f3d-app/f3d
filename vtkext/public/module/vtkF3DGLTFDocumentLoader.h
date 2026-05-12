#ifndef vtkF3DGLTFDocumentLoader_h
#define vtkF3DGLTFDocumentLoader_h

#include <vtkGLTFDocumentLoader.h>

class vtkF3DGLTFDocumentLoader : public vtkGLTFDocumentLoader
{
public:
  static vtkF3DGLTFDocumentLoader* New();
  vtkTypeMacro(vtkF3DGLTFDocumentLoader, vtkGLTFDocumentLoader);

  std::vector<std::string> GetSupportedExtensions() override;

  void PrepareData() override;

protected:
  vtkF3DGLTFDocumentLoader() = default;
  ~vtkF3DGLTFDocumentLoader() override = default;

private:
  vtkF3DGLTFDocumentLoader(const vtkF3DGLTFDocumentLoader&) = delete;
  void operator=(const vtkF3DGLTFDocumentLoader&) = delete;
};

#endif