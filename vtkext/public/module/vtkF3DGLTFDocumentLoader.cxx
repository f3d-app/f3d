#include "vtkF3DGLTFDocumentLoader.h"
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DGLTFDocumentLoader);

//----------------------------------------------------------------------------
std::vector<std::string> vtkF3DGLTFDocumentLoader::GetSupportedExtensions()
{
  std::vector<std::string> extensions = this->Superclass::GetSupportedExtensions();
  extensions.emplace_back("EXT_texture_webp");
  return extensions;
}