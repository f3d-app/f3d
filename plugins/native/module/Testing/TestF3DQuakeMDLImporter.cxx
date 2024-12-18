#include "vtkF3DQuakeMDLImporter.h"
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DQuakeMDLImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/glaunch.mdl";
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
  return numAnimations == 2 ? EXIT_SUCCESS : EXIT_FAILURE;
}