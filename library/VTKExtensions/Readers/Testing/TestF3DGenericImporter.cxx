#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DReaderInstantiator.h"
#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  F3DReaderInstantiator ReaderInstantiator;
  
  vtkNew<vtkF3DGenericImporter> importer;

  // Test invalid reader filepath
  std::string dummyFilename = std::string(argv[1]) + "data/foo.dummy";
  importer->SetFileName(dummyFilename);
  if (importer->CanReadFile())
  {
    std::cerr << "Importer unexpectedly can read a non-existent file" << std::endl;
    return EXIT_FAILURE;
  }
  importer->Update();
  importer->Print(cout);

  // Test valid reader filepath
  std::string filename = std::string(argv[1]) + "data/small.ex2";
  importer->SetFileName(filename);
  if (!importer->CanReadFile())
  {
    std::cerr << "Importer unexpectedly can not read a valid file" << std::endl;
    return EXIT_FAILURE;
  }

  importer->Update();
  importer->Print(cout);
  if (importer->GetNumberOfAnimations() != 1)
  {
    std::cerr << "Unexpected number of animations" << std::endl;
    return EXIT_FAILURE;
  }

  importer->EnableAnimation(0);
  if (!importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected not enabled animation" << std::endl;
    return EXIT_FAILURE;
  }

  importer->DisableAnimation(0);
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enabled animation" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
