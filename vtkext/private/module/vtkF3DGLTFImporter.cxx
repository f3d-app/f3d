#include "vtkF3DGLTFImporter.h"

#include <vtkActor.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLShaderProperty.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFImporter);

//----------------------------------------------------------------------------
vtkF3DGLTFImporter::vtkF3DGLTFImporter()
{
  this->ImportArmature = true;
}

//----------------------------------------------------------------------------
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20241220) // To update when merged
void vtkF3DGLTFImporter::ApplyArmatureProperties(vtkActor* actor)
{
  this->Superclass::ApplyArmatureProperties(actor);

  // armature on top
  vtkOpenGLShaderProperty* sp = vtkOpenGLShaderProperty::SafeDownCast(actor->GetShaderProperty());
  if (sp)
  {
    // compress the depth value to the first thousand slice in front of the camera to make sure it's displayed on top
    sp->AddFragmentShaderReplacement("//VTK::Depth::Impl", false, "gl_FragDepth = gl_FragDepth * 0.001;\n", false);
  }
}
#endif
