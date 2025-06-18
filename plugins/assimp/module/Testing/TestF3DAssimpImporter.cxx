#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include "vtkF3DAssimpImporter.h"

#include <iostream>

int TestF3DAssimpImporter(int vtkNotUsed(argc), char* argv[])
{
  vtkNew<vtkRenderWindow> window;
  vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);

  std::string filename = std::string(argv[1]) + "data/animatedWorld.fbx";
  vtkNew<vtkF3DAssimpImporter> importer;
  importer->SetFileName(filename);
  importer->SetRenderWindow(window);
  importer->Update();
  importer->Print(cout);
  return importer->GetNumberOfAnimations() == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
