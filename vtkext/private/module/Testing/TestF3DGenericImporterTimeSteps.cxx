#include "vtkF3DGenericImporter.h"

#include <vtkDataArrayRange.h>
#include <vtkDoubleArray.h>
#include <vtkHDFReader.h>
#include <vtkNew.h>

#include <format>
#include <iostream>

int TestF3DGenericImporterTimeSteps(int argc, char* argv[])
{
  // Test time step temporal information
  {
    vtkNew<vtkHDFReader> reader;
    const std::string filename = std::format("{}data/blob.vtkhdf", argv[1]);
    reader->SetFileName(filename.c_str());
    reader->UpdateInformation();

    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetInternalReader(reader);
    importer->Update();
    importer->Print(std::cout);

    int nbTimeSteps;
    std::array<double, 2> timeRange{};
    vtkNew<vtkDoubleArray> timeSteps;

    const bool temporalInfoExists =
      importer->GetTemporalInformation(0, timeRange.data(), nbTimeSteps, timeSteps);
    if (!temporalInfoExists)
    {
      std::cerr << "Unexpected return value with GetTemporalInformation\n";
      return EXIT_FAILURE;
    }

    const bool validTimeStep{
      nbTimeSteps >= 1 && nbTimeSteps == timeSteps->GetNumberOfTuples() &&
      nbTimeSteps == reader->GetNumberOfSteps() && nbTimeSteps == 11 // 11 timesteps in blob.vtkhdf
    };

    if (!validTimeStep)
    {
      std::cerr << "Unexpected time steps recovered\n";
      return EXIT_FAILURE;
    }

    const auto values = vtk::DataArrayValueRange(timeSteps);
    const bool allInRange = std::ranges::all_of(
      values, [&](const double t) { return t >= timeRange[0] && t <= timeRange[1]; });

    if (!allInRange)
    {
      std::cerr << "Unexpected time step in time range\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
