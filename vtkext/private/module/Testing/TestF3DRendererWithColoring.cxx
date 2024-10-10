#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkRenderWindow.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkF3DConfigure.h"
#include "vtkF3DGenericImporter.h"
#include "vtkF3DRenderer.h"

int TestF3DRendererWithColoring(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderer> renderer;
  vtkNew<vtkF3DMetaImporter> importer;
  vtkNew<vtkRenderWindow> window;

  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);
  renderer->SetImporter(importer);

  // Check invalid bounding box code path
  renderer->ShowGrid(true);
  renderer->UpdateActors();

  vtkNew<vtkXMLUnstructuredGridReader> readerVTU;
  std::string filename = std::string(argv[1]) + "data/bluntfin_t.vtu";
  readerVTU->SetFileName(filename.c_str());
  vtkNew<vtkF3DGenericImporter> importerVTU;
  importerVTU->SetInternalReader(readerVTU);

  importer->AddImporter(importerVTU);
  importer->Update();

  // Check invalid array code path
  renderer->SetColoring(true, false, "Invalid", 0);
  renderer->SetUseVolume(false);
  renderer->UpdateActors();
  renderer->CycleScalars(vtkF3DRenderer::CycleType::COMPONENT);
  renderer->SetUseVolume(true);
  renderer->UpdateActors();

  if (renderer->GetColoringArrayName() != "Density" || renderer->GetColoringComponent() != 0)
  {
    std::cerr << "Unexpected coloring information with invalid array" << std::endl;
    return EXIT_FAILURE;
  }

  // Check invalid component code path
  renderer->SetColoring(true, false, "Momentum", 5);
  renderer->UpdateActors();
  renderer->SetUseVolume(true);
  renderer->UpdateActors();

  if (renderer->GetColoringArrayName() != "Momentum" || renderer->GetColoringComponent() != 5)
  {
    std::cerr << "Unexpected coloring information with invalid component" << std::endl;
    return EXIT_FAILURE;
  }

  renderer->CycleScalars(vtkF3DRenderer::CycleType::COMPONENT);
  if (renderer->GetColoringArrayName() != "Momentum" || renderer->GetColoringComponent() != 1)
  {
    std::cerr << "Unexpected coloring information after cycling component" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
