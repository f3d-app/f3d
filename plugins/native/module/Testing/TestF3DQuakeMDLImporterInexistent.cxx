#include <vtkNew.h>

#include "vtkF3DQuakeMDLImporter.h"

#include <iostream>

int TestF3DQuakeMDLImporterInexistent(int vtkNotUsed(argc), char* argv[])
{
  std::string pathInexistent = std::string(argv[1]) + "data/inexistent.mdl";
  vtkNew<vtkF3DQuakeMDLImporter> importer;
  importer->SetFileName(pathInexistent.c_str());
  if (importer->Update())
  {
    std::cerr << "Unexpected success opening inexistent file\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
