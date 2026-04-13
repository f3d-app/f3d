#include "vtkF3DGLTFImporter.h"

#include "vtkF3DImporter.h"

#include <vtkActor.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>

// need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 20260409)
#include <vtkActorCollection.h>
#include <vtkProperty.h>

#include <cmath>
#endif

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

// need https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13116
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 6, 20260409)
//----------------------------------------------------------------------------
void vtkF3DGLTFImporter::ImportActors(vtkRenderer* renderer)
{
  this->Superclass::ImportActors(renderer);

  // loop on actors
  vtkCollectionSimpleIterator ait;
  this->ActorCollection->InitTraversal(ait);
  while (vtkActor* actor = this->ActorCollection->GetNextActor(ait))
  {
    vtkProperty* prop = actor->GetProperty();
    if (prop->GetLighting() == false)
    {
      double color[3];
      prop->GetColor(color);

      // convert to linear space
      auto toLinear = [](double c) { return std::pow(c, 2.2); };
      color[0] = toLinear(color[0]);
      color[1] = toLinear(color[1]);
      color[2] = toLinear(color[2]);
      prop->SetColor(color);
    }
  }
}
#endif
