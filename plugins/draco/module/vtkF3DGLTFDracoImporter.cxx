#include "vtkF3DGLTFDracoImporter.h"

#include "vtkF3DGLTFDracoDocumentLoader.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFDracoImporter);

//----------------------------------------------------------------------------
void vtkF3DGLTFDracoImporter::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDracoDocumentLoader>::New();
}
