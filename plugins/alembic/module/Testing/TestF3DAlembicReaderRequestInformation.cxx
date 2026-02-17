#include <vtkAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAlembicReader.h"

#include <iostream>

int TestF3DAlembicReaderRequestInformation(int vtkNotUsed(argc), char* argv[])
{
  static int NUMBER_OF_TIME_STEPS = 90;
  static const double expectedTimeSteps[] = { 0.000000, 0.033333, 0.066667, 0.100000, 0.133333,
    0.166667, 0.200000, 0.233333, 0.266667, 0.300000, 0.333333, 0.366667, 0.400000, 0.433333,
    0.466667, 0.500000, 0.533333, 0.566667, 0.600000, 0.633333, 0.666667, 0.700000, 0.733333,
    0.766667, 0.800000, 0.833333, 0.866667, 0.900000, 0.933333, 0.966667, 1.000000, 1.033333,
    1.066667, 1.100000, 1.133333, 1.166667, 1.200000, 1.233333, 1.266667, 1.300000, 1.333333,
    1.366667, 1.400000, 1.433333, 1.466667, 1.500000, 1.533333, 1.566667, 1.600000, 1.633333,
    1.666667, 1.700000, 1.733333, 1.766667, 1.800000, 1.833333, 1.866667, 1.900000, 1.933333,
    1.966667, 2.000000, 2.033333, 2.066667, 2.100000, 2.133333, 2.166667, 2.200000, 2.233333,
    2.266667, 2.300000, 2.333333, 2.366667, 2.400000, 2.433333, 2.466667, 2.500000, 2.533333,
    2.566667, 2.600000, 2.633333, 2.666667, 2.700000, 2.733333, 2.766667, 2.800000, 2.833333,
    2.866667, 2.900000, 2.933333, 2.966667 };
  static const double expectedTimeRange[] = { 0.000000, 2.966667 };

  std::string filename = std::string(argv[1]) + "data/drop.abc";
  vtkNew<vtkF3DAlembicReader> reader;
  reader->SetFileName(filename);
  reader->UpdateInformation();
  vtkInformation* readerInfo = reader->GetOutputInformation(0);
  double* readerTimeSteps = nullptr;
  if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
  {
    readerTimeSteps = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  }
  else
  {
    return EXIT_FAILURE;
  }
  double* readerTimeRange = nullptr;
  if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()))
  {
    readerTimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
  }
  else
  {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < 2; i++)
  {
    int a = fabs(readerTimeRange[i] - expectedTimeRange[i]);
    if (fabs(readerTimeRange[i] - expectedTimeRange[i]) > 1e-6)
    {
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < NUMBER_OF_TIME_STEPS; i++)
  {
    if (fabs(readerTimeSteps[i] - expectedTimeSteps[i]) > 1e-6)
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
