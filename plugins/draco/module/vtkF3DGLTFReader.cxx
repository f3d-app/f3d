#include "vtkF3DGLTFReader.h"

#include "vtkF3DGLTFDocumentLoader.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFReader);

//----------------------------------------------------------------------------
void vtkF3DGLTFReader::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDocumentLoader>::New();
}
