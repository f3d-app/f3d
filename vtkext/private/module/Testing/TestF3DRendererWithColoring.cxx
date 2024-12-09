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
  renderer->SetEnableColoring(true);
  renderer->SetArrayNameForColoring("Invalid");
  renderer->SetUseVolume(false);
  renderer->UpdateActors();
  renderer->CycleComponentForColoring();
  renderer->SetUseVolume(true);
  renderer->UpdateActors();

  if (renderer->GetArrayNameForColoring() != "Invalid" || renderer->GetComponentForColoring() != -1)
  {
    std::cerr << "Unexpected coloring information with invalid array" << std::endl;
    return EXIT_FAILURE;
  }

  // Check invalid component code path
  renderer->SetArrayNameForColoring("Momentum");
  renderer->SetComponentForColoring(5);
  renderer->UpdateActors();
  renderer->SetUseVolume(true);
  renderer->UpdateActors();

  if (renderer->GetArrayNameForColoring() != "Momentum" || renderer->GetComponentForColoring() != 5)
  {
    std::cerr << "Unexpected coloring information with invalid component" << std::endl;
    return EXIT_FAILURE;
  }

  renderer->CycleComponentForColoring();
  if (renderer->GetArrayNameForColoring() != "Momentum" || renderer->GetComponentForColoring() != 1)
  {
    std::cerr << "Unexpected coloring information after cycling component" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
