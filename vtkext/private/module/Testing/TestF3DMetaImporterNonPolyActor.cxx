#include "vtkF3DMetaImporter.h"

#include <vtkCubeSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// NonPolyImporter : Testing class which creates 2 actors, one with polydata (a default cube) and
// one with no poly data.

class NonPolyImporter : public vtkImporter
{
public:
  static NonPolyImporter* New();
  vtkTypeMacro(NonPolyImporter, vtkImporter);

  void ImportActors(vtkRenderer* renderer) override
  {
    // Create poly / non-poly containing actors
    vtkNew<vtkActor> actor1;
    vtkNew<vtkActor> actor2;
    {
      vtkNew<vtkCubeSource> cube;
      vtkNew<vtkPolyDataMapper> pmap;
      pmap->SetInputConnection(cube->GetOutputPort());
      actor1->SetMapper(pmap);
    }
    //
    renderer->AddActor(actor1);
    renderer->AddActor(actor2);

    // Push to vtkImporter parent class
    this->ActorCollection->AddItem(actor1);
    this->ActorCollection->AddItem(actor2);
  }
};

vtkStandardNewMacro(NonPolyImporter);

//

int TestF3DMetaImporterNonPolyActor(int argc, char* argv[])
{
  // Setup importers with testing class
  vtkNew<vtkF3DMetaImporter> importer;
  vtkNew<NonPolyImporter> importerNP;
  importer->AddImporter({ "foo", importerNP });

  // Try and render image.
  vtkNew<vtkRenderWindow> window;
  vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);
  // F3D will crash here if it cannot handle a non-poly containing actor.
  importer->Update();
  importer->GetColoringInfoHandler();

  return EXIT_SUCCESS;
}
