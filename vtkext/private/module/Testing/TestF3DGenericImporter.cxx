#include <vtkDoubleArray.h>
#include <vtkGLTFReader.h>
#include <vtkNew.h>
#include <vtkPLYReader.h>
#include <vtkTestUtilities.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  vtkNew<vtkF3DGenericImporter> importer;

  // Test without any reader
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

  int nbTimeSteps;
  double timeRange[2];
  vtkNew<vtkDoubleArray> timeSteps;

  if (importer->GetTemporalInformation(0, 60, nbTimeSteps, timeRange, timeSteps))
  {
    std::cerr << "Unexpected return value with GetTemporalInformation" << std::endl;
    return EXIT_FAILURE;
  }

  // Test with reader
  vtkNew<vtkGLTFReader> reader;

  std::string filename = std::string(argv[1]) + "data/BoxAnimated.gltf";
  reader->SetFileName(filename.c_str());
  reader->UpdateInformation();
  reader->EnableAnimation(0);

  importer->AddInternalReader("Test", reader);
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

  // Test with multiple readers
  vtkNew<vtkPLYReader> reader2;
  filename = std::string(argv[1]) + "data/suzanne.ply";
  reader2->SetFileName(filename.c_str());
  importer->AddInternalReader("Test2", reader2);

  if (!importer->CanReadFile())
  {
    std::cerr << "Importer unexpectedly can not read a valid file" << std::endl;
    return EXIT_FAILURE;
  }
  importer->Update();
  std::string description = importer->GetMetaDataDescription();
  if (description.find("Number of geometries: 2") == std::string::npos)
  {
    std::cerr << "Unexpected meta data description with multiple geometries" << std::endl;
    return EXIT_FAILURE;
  }

  // Static method testing
  if (vtkF3DGenericImporter::GetDataObjectDescription(nullptr) != "")
  {
    std::cerr << "Unexpected data object description with null input" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
