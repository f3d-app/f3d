#include "vtkF3DGenericImporter.h"

#include <vtkDoubleArray.h>
#include <vtkHDFReader.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DGenericImporterTimeSteps(int argc, char* argv[])
{
  // Test time step temporal information
  {
    vtkNew<vtkHDFReader> reader;
    std::string filename = std::string(argv[1]) + "data/blob.vtkhdf";
    reader->SetFileName(filename.c_str());
    reader->UpdateInformation();

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader);
    importer->Update();
    importer->Print(std::cout);

    int nbTimeSteps;
    double timeRange[2];
    vtkNew<vtkDoubleArray> timeSteps;

    bool temporalInfoExists =
      importer->GetTemporalInformation(0, timeRange, nbTimeSteps, timeSteps);
    if (!temporalInfoExists)
    {
      std::cerr << "Unexpected return value with GetTemporalInformation\n";
      return EXIT_FAILURE;
    }

    if (nbTimeSteps < 1 || nbTimeSteps != timeSteps->GetNumberOfTuples())
    {
      std::cerr << "Unexpected number of timesteps\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
