#include "vtkF3DUSDImporter.h"

#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkVersion.h>

#include <iostream>

int TestF3DUSDImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.usd";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->DisableAnimation(0);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 4, 20250507)
  if (importer->GetAnimationSupportLevel() != vtkImporter::AnimationSupportLevel::UNIQUE)
  {
    return EXIT_FAILURE;
  }
#endif

  importer->Update();
  importer->Print(cout);
  return importer->GetRenderer() ? EXIT_SUCCESS : EXIT_FAILURE;
}
