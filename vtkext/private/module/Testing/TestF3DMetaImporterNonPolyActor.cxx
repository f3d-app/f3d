#include "vtkF3DMetaImporter.h"

#include <vtkCubeSource.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkVRMLImporter.h>

// NonPolyImporter : Testing class which creates 2 actors, one with polydata (a default cube) and
// one with no poly data.

class NonPolyImporter : public vtkImporter
{
public:
  static NonPolyImporter* New();
  vtkTypeMacro(NonPolyImporter, vtkImporter);

private:
  NonPolyImporter()
  {
    // Create poly / non-poly containing actors
    vtkNew<vtkActor> actor1;
    {
      vtkCubeSource* cube = vtkCubeSource::New();
      vtkNew<vtkPolyDataMapper> pmap;
      pmap->SetInputConnection(cube->GetOutputPort());
      actor1->SetMapper(pmap);
    }
    vtkNew<vtkActor> actor2;

    // Push to vtkImporter parent class
    auto collection = this->GetImportedActors();
    collection->AddItem(actor1);
    collection->AddItem(actor2);
  }
};

vtkStandardNewMacro(NonPolyImporter);

//

int TestF3DMetaImporterNonPolyActor(int argc, char* argv[])
{
  // Setup importers with testing class
  vtkNew<vtkF3DMetaImporter> importer;
  vtkNew<NonPolyImporter> importerNP;
  importer->AddImporter(importerNP);

  // Try and render image. F3D will crash if it cannot handle a non-poly containing actor.
  vtkNew<vtkRenderWindow> window;
  vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);
  importer->Update();

  return EXIT_SUCCESS;
}
