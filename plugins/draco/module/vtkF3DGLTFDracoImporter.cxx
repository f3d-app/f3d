#include "vtkF3DGLTFDracoImporter.h"

#include "vtkF3DGLTFDracoDocumentLoader.h"

#include <vtkFileResourceStream.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFDracoImporter);

//----------------------------------------------------------------------------
void vtkF3DGLTFDracoImporter::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDracoDocumentLoader>::New();
}

//------------------------------------------------------------------------------
bool vtkF3DGLTFDracoImporter::CanReadFile(vtkResourceStream* stream)
{
  if (!stream)
  {
    return false;
  }

  stream->Seek(0, vtkResourceStream::SeekDirection::Begin);
  vtkNew<vtkF3DGLTFDracoDocumentLoader> loader;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260212)
  if (!loader->LoadModelMetaDataFromStream(stream, nullptr, true))
#else
  if (!loader->LoadModelMetaDataFromStream(stream))
#endif
  {
    return false;
  }
  return true;
}
