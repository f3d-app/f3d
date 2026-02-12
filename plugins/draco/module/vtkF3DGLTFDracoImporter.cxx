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
bool vtkF3DGLTFDracoImporter::CanReadFile(const std::string& filename)
{
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    return false;
  }
  return vtkF3DGLTFDracoImporter::CanReadFile(stream);
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
  if (!loader->LoadModelMetaDataFromStream(stream, nullptr, true))
  {
    return false;
  }
  return true;
}
