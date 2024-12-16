#include "vtkF3DGLTFImporter.h"

#include "vtkF3DRenderPass.h"

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
  info->Set(vtkF3DRenderPass::ACTOR_IS_ARMATURE(), 1);
  actor->SetPropertyKeys(info);

  // armature on top
  //vtkOpenGLShaderProperty* sp = vtkOpenGLShaderProperty::SafeDownCast(actor->GetShaderProperty());
  //if (sp)
  //{
  //  // compress the depth value to the first thousandth slice in front of the camera to make sure it's displayed on top
  //  sp->AddFragmentShaderReplacement("//VTK::Depth::Impl", false, "gl_FragDepth = gl_FragDepth * 0.001;\n", false);
  //}
}
#endif
