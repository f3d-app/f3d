#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAlembicImporter.h"

#include <iostream>

int TestF3DAlembicImporter(int argc, char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.abc";
  vtkNew<vtkF3DAlembicImporter> importer;
  importer->SetFileName(filename);
  importer->Update();
  importer->Print(cout);
  return importer->GetRenderer() ? EXIT_SUCCESS : EXIT_FAILURE;
}
