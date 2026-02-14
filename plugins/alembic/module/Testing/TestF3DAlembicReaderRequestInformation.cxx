#include <vtkNew.h>
#include <vtkAlgorithm.h>
#include <vtkTestUtilities.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "vtkF3DAlembicReader.h"

#include <iostream>

int TestF3DAlembicReaderRequestInformation(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/drop.abc";
  vtkNew<vtkF3DAlembicReader> reader;
  reader->SetFileName(filename);
  reader->UpdateInformation();
  vtkInformation* readerInfo = reader->GetOutputInformation(0);
  double* readerTimeSteps = nullptr;
  if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS())) {
    readerTimeSteps = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  } else {
    return EXIT_FAILURE;
  }
  double* readerTimeRange = nullptr;
  if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE())) {
    readerTimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
  } else {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
