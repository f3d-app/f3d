#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DUSDImporter.h"

#include <iostream>

int TestF3DUSDImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.usd";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename);
  importer->DisableAnimation(0);
  importer->Update();
  importer->Print(cout);
  return importer->GetRenderer() ? EXIT_SUCCESS : EXIT_FAILURE;
}
