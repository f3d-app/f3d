#include "vtkF3DGLTFImporter.h"

#include "vtkF3DGLTFDocumentLoader.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFImporter);

//----------------------------------------------------------------------------
void vtkF3DGLTFImporter::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDocumentLoader>::New();
}
