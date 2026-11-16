#include "vtkF3DUSDImporter.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

bool nearlyEqual(double actual, double expected)
{
  constexpr double tolerance = 1e-9;
  return std::isfinite(actual) && std::abs(actual - expected) <= tolerance;
};

int TestF3DUSDImporterTimeInformation(int vtkNotUsed(argc), char* argv[])
{
  const std::string filename = std::string(argv[1]) + "data/timesteps.usda";
  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetFileName(filename.c_str());
  importer->Update();

  // not used, yet
  int animationIndex = 0;
  vtkNew<vtkDoubleArray> AnimationTimeSteps;
  double timeRange[2];
  int nbTimeSteps;

  bool retValue =
    importer->GetTemporalInformation(animationIndex, timeRange, nbTimeSteps, AnimationTimeSteps);

  if (!retValue)
  {
    std::cerr << "Method GetTemporalInformation failed\n";
    return EXIT_FAILURE;
  }

  constexpr int expectedCount = 4;
  constexpr double expectedTimeSteps[expectedCount] = { 0.0, 0.5, 1.0, 2.0 };

  if (nbTimeSteps != expectedCount)
  {
    std::cerr << "Expected nbTimeSteps = " << expectedCount << ", got " << nbTimeSteps << '\n';
    return EXIT_FAILURE;
  }

  if (AnimationTimeSteps->GetNumberOfComponents() != 1)
  {
    std::cerr << "Expected one component per time step\n";
    return EXIT_FAILURE;
  }

  if (AnimationTimeSteps->GetNumberOfTuples() != expectedCount)
  {
    std::cerr << "Expected " << expectedCount << " time-step tuples, got "
              << AnimationTimeSteps->GetNumberOfTuples() << '\n';
    return EXIT_FAILURE;
  }

  for (int i = 0; i < expectedCount; ++i)
  {
    const double actual = AnimationTimeSteps->GetValue(i);
    if (!nearlyEqual(actual, expectedTimeSteps[i]))
    {
      std::cerr << "Time step " << i << ": expected " << expectedTimeSteps[i] << ", got " << actual
                << '\n';
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
