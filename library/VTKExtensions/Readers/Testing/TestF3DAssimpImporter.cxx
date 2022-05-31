#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAssimpImporter.h"

#include <iostream>

int TestF3DAssimpImporter(int argc, char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/robot_kyle_walking.fbx";
  vtkNew<vtkF3DAssimpImporter> importer;
  importer->SetFileName(filename);
  importer->Update();
  importer->Print(cout);
  return importer->GetNumberOfAnimations() == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
