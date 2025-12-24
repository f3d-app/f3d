#include "vtkF3DGLTFImporter.h"

#include "vtkF3DImporter.h"

#include <vtkActor.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLShaderProperty.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFImporter);

//----------------------------------------------------------------------------
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20241219)
vtkF3DGLTFImporter::vtkF3DGLTFImporter()
{
  this->SetImportArmature(true);
}
#else
vtkF3DGLTFImporter::vtkF3DGLTFImporter() = default;
#endif

//----------------------------------------------------------------------------
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20241219)
void vtkF3DGLTFImporter::ApplyArmatureProperties(vtkActor* actor)
{
  this->Superclass::ApplyArmatureProperties(actor);

  vtkNew<vtkInformation> info;
  info->Set(vtkF3DImporter::ACTOR_IS_ARMATURE(), 1);
  actor->SetPropertyKeys(info);
}
#endif
