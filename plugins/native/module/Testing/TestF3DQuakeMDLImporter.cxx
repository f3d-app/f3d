#include "vtkF3DQuakeMDLImporter.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DQuakeMDLImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string filename =
    std::string(argv[1]) + "data/zombie_2.mdl"; // MDL can load two types of frames and two types of textures,
                                                // another file was added for coverage.  
  vtkNew<vtkF3DQuakeMDLImporter> importer;
  importer->SetFileName(filename);
  importer->Update();
  importer->Print(cout);
  vtkIdType numAnimations = importer->GetNumberOfAnimations();
  for (int i = 0; i < numAnimations; i++)
  {
    importer->DisableAnimation(i);
  }
  vtkIdType selectedAnimationIndex = 0;
  importer->EnableAnimation(selectedAnimationIndex);
  std::string animationName = importer->GetAnimationName(0);
  return numAnimations == 1 && animationName == "frame" ? EXIT_SUCCESS : EXIT_FAILURE;
}
