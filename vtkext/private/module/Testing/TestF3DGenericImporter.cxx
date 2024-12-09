#include <vtkDoubleArray.h>
#include <vtkGLTFReader.h>
#include <vtkNew.h>
#include <vtkPLYReader.h>
#include <vtkTestUtilities.h>
#include <vtkVersion.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporter(int argc, char* argv[])
{
  vtkNew<vtkF3DGenericImporter> importer;
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

  importer->SetInternalReader(reader);
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

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240910)
  filename = std::string(argv[1]) + "data/BoxAnimated_invalid_animation.gltf";
  reader->SetFileName(filename.c_str());
  reader->UpdateInformation();
  reader->EnableAnimation(0);

  importer->SetInternalReader(reader);
  importer->Update();
  if (importer->UpdateAtTimeValue(0.1))
  {
    std::cerr << "Unexpected UpdateAtTimeValue success" << std::endl;
    return EXIT_FAILURE;
  }
#endif

  // Static method testing
  if (vtkF3DGenericImporter::GetDataObjectDescription(nullptr) != "")
  {
    std::cerr << "Unexpected data object description with null input" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
