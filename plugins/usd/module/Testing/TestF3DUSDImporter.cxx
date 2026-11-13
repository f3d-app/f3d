#include "vtkF3DUSDImporter.h"

#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DUSDImporter(int vtkNotUsed(argc), char* argv[])
{
  const std::string filename = std::string(argv[1]) + "data/suzanne.usd";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->DisableAnimation(0);

  if (importer->GetAnimationSupportLevel() != vtkImporter::AnimationSupportLevel::UNIQUE)
  {
    return EXIT_FAILURE;
  }

  importer->Update();
  importer->Print(std::cout);
  return importer->GetRenderer() ? EXIT_SUCCESS : EXIT_FAILURE;
}
