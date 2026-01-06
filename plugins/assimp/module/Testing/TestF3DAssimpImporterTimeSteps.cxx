#include "vtkF3DAssimpImporter.h"

#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkDoubleArray.h>

#include <iostream>

int TestF3DAssimpImporterTimeSteps(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/anim_test.x";
  vtkNew<vtkF3DAssimpImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->Update();

  double timeRange[2];
  int nbTimeSteps;
  vtkNew<vtkDoubleArray> timeSteps;
  importer->GetTemporalInformation(0, timeRange, nbTimeSteps, timeSteps);
  importer->Print(std::cout);

  if (nbTimeSteps != 24) 
  {
    std::cerr << "Time steps is not as expected.\n";
    return EXIT_FAILURE;
  }

  for (int i = 0; i < nbTimeSteps; i++) 
  {
    double currentTimeStep = timeSteps->GetValue(i);
    if (currentTimeStep > timeRange[1] || currentTimeStep < timeRange[0])
    {
      std::cerr << "Time step is out of boundary. \n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
