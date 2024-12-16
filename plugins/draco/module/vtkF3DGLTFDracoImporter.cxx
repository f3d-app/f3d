#include "vtkF3DDracoGLTFImporter.h"

#include "vtkF3DGLTFDracoDocumentLoader.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DDracoGLTFImporter);

//----------------------------------------------------------------------------
void vtkF3DDracoGLTFImporter::InitializeLoader()
{
  this->Loader = vtkSmartPointer<vtkF3DGLTFDracoDocumentLoader>::New();
}
