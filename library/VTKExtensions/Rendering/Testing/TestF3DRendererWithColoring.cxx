#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkF3DGenericImporter.h"
#include "vtkF3DRendererWithColoring.h"

int TestF3DRendererWithColoring(int argc, char* argv[])
{
  vtkNew<vtkF3DRendererWithColoring> renderer;
  vtkNew<vtkF3DGenericImporter> importer;
  renderer->SetImporter(importer);

  // Read a vts and a vtu with same array names
  // but different component names and array ranges
  vtkNew<vtkXMLUnstructuredGridReader> readerVTU;
  std::string filename = std::string(argv[1]) + "data/bluntfin_t.vtu";
  readerVTU->SetFileName(filename.c_str());
  importer->AddInternalReader("VTU", readerVTU);
  importer->Update();

  // Check invalid array code path
  renderer->SetColoring(false, "Invalid", 0);
  renderer->SetUseVolume(false);
  renderer->UpdateColoringActors();
  renderer->CycleScalars(vtkF3DRendererWithColoring::CycleType::COMPONENT);
  renderer->SetUseVolume(true);
  renderer->UpdateColoringActors();

  if (renderer->GetColoringArrayName() != "Density" || renderer->GetColoringComponent() != 0)
  {
    std::cerr << "Unexpected coloring information with invalid array" << std::endl;
    return EXIT_FAILURE;
  }

  // Check invalid component code path
  renderer->SetColoring(false, "Momentum", 5);
  renderer->UpdateColoringActors();
  renderer->SetUseVolume(true);
  renderer->UpdateColoringActors();

  if (renderer->GetColoringArrayName() != "Momentum" || renderer->GetColoringComponent() != 5)
  {
    std::cerr << "Unexpected coloring information with invalid component" << std::endl;
    return EXIT_FAILURE;
  }

  renderer->CycleScalars(vtkF3DRendererWithColoring::CycleType::COMPONENT);
  if (renderer->GetColoringArrayName() != "Momentum" || renderer->GetColoringComponent() != 1)
  {
    std::cerr << "Unexpected coloring information after cycling component" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
