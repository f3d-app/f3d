#include "vtkF3D3DSImporter.h"

#include <vtkObjectFactory.h>
#include <vtkProperty.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3D3DSImporter);

vtkF3D3DSImporter::vtkF3D3DSImporter()
  : vtk3DSImporter()
{
  this->ComputeNormals = true;
}

//----------------------------------------------------------------------------
void vtkF3D3DSImporter::ImportProperties(vtkRenderer* vtkNotUsed(renderer))
{
  // override default behavior to setup PBR materials
  for (vtk3DSMatProp* m = this->MatPropList; m != nullptr; m = static_cast<vtk3DSMatProp*>(m->next))
  {
    auto toLinear = [](float value) { return pow(value, 2.2); };

    m->aProperty->SetInterpolationToPBR();
    m->aProperty->SetColor(
      toLinear(m->diffuse.red), toLinear(m->diffuse.green), toLinear(m->diffuse.blue));
    m->aProperty->SetOpacity(1.0 - m->transparency);
  }
}
