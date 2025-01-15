#include "vtkF3DQuakeMDLImporter.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DQuakeMDLImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename =
    std::string(argv[1]) + "data/glaunch_2.mdl"; // File was modified to add coverage.
  vtkNew<vtkF3DQuakeMDLImporter> importer;
  importer->SetFileName(filename);
  importer->Update();
  importer->Print(cout);
  vtkIdType numAnimations = importer->GetNumberOfAnimations();
  for (int i = 0; i < numAnimations; i++)
  {
    importer->DisableAnimation(i);
  }
  vtkIdType selectedAnimationIndex = 1;
  importer->EnableAnimation(selectedAnimationIndex);
  std::string animationName = importer->GetAnimationName(0);
  return numAnimations == 2 && animationName == "stand" ? EXIT_SUCCESS : EXIT_FAILURE;
}
