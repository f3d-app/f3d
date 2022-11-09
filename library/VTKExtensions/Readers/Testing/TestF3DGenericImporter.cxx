#include <vtkGLTFReader.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  vtkNew<vtkF3DGenericImporter> importer;

  if (importer->CanReadFile())
  {
    std::cerr << "Importer unexpectedly can read a file without internal reader" << std::endl;
    return EXIT_FAILURE;
  }
  importer->Update();
  importer->Print(cout);
  if (importer->GetNumberOfAnimations() != 0)
  {
    std::cerr << "Unexpected number of animations" << std::endl;
    return EXIT_FAILURE;
  }

  // Test valid reader
  vtkNew<vtkGLTFReader> reader;

  std::string filename = std::string(argv[1]) + "data/BoxAnimated.gltf";
  reader->SetFileName(filename.c_str());
  reader->UpdateInformation();
  reader->EnableAnimation(0);

  importer->SetInternalReader(reader);
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
