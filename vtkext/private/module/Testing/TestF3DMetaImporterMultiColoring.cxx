#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTestUtilities.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkF3DGenericImporter.h"
#include "vtkF3DMetaImporter.h"

#include <iostream>

int TestF3DMetaImporterMultiColoring(int argc, char* argv[])
{
  vtkNew<vtkF3DMetaImporter> importer;

  // Check importer error code path
  if (!importer->GetAnimationName(0).empty())
  {
    std::cerr << "Unexpected animation name that should be empty" << std::endl;
    return EXIT_FAILURE;
  }
  if (importer->IsAnimationEnabled(0))
  {
    std::cerr << "Unexpected enabled animation that should not be" << std::endl;
    return EXIT_FAILURE;
  }

  int nbTimeSteps;
  double timeRange[2];
  if (importer->GetTemporalInformation(0, 60, nbTimeSteps, timeRange, nullptr))
  {
    std::cerr << "Unexpected enabled animation that should not be" << std::endl;
    return EXIT_FAILURE;
  }

  if (!importer->GetCameraName(0).empty())
  {
    std::cerr << "Unexpected camera name that should be empty" << std::endl;
    return EXIT_FAILURE;
  }

  if (importer->FindIndexForColoring(false, "") != -1)
  {
    std::cerr << "Unexpected FindIndexForColoring success" << std::endl;
    return EXIT_FAILURE;
  }


  // Read a vts and a vtu with same array names
  // but different component names and array ranges
  vtkNew<vtkXMLUnstructuredGridReader> readerVTU;
  std::string filename = std::string(argv[1]) + "data/bluntfin_t.vtu";
  readerVTU->SetFileName(filename.c_str());
  vtkNew<vtkF3DGenericImporter> importerVTU;
  importerVTU->SetInternalReader(readerVTU);

  vtkNew<vtkXMLStructuredGridReader> readerVTS;
  filename = std::string(argv[1]) + "data/bluntfin.vts";
  readerVTS->SetFileName(filename.c_str());
  vtkNew<vtkF3DGenericImporter> importerVTS;
  importerVTS->SetInternalReader(readerVTS);

  importer->AddImporter(importerVTU);
  importer->AddImporter(importerVTS);

  vtkNew<vtkRenderWindow> window;
  vtkNew<vtkRenderer> renderer;
  window->AddRenderer(renderer);
  importer->SetRenderWindow(window);
  importer->Update();

  if (importer->GetNumberOfIndexesForColoring(false) != 3)
  {
    std::cerr << "Importer provide unexpected number of indexes for coloring" << std::endl;
    return EXIT_FAILURE;
  }

  int idx = importer->FindIndexForColoring(false, "Momentum");
  if (idx < 0)
  {
    std::cerr << "Importer unable to find an expected coloring array" << std::endl;
    return EXIT_FAILURE;
  }

  vtkF3DMetaImporter::ColoringInfo info;
  importer->GetInfoForColoring(false, idx, info);
  if (info.Name != "Momentum")
  {
    std::cerr << "Unexpected coloring name: " << info.Name << std::endl;
    return EXIT_FAILURE;
  }
  if (info.MaximumNumberOfComponents != 3)
  {
    std::cerr << "Unexpected coloring nComp" << std::endl;
    return EXIT_FAILURE;
  }
  if (info.ComponentNames[0] != "LX Momentum")
  {
    std::cerr << "Unexpected coloring component name 0" << std::endl;
    return EXIT_FAILURE;
  }
  if (info.ComponentNames[1] != "LY Momentum_t")
  {
    std::cerr << "Unexpected coloring component name 1" << std::endl;
    return EXIT_FAILURE;
  }
  if (info.ComponentNames[2] != "")
  {
    std::cerr << "Unexpected coloring component name 2" << std::endl;
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(info.ComponentRanges[0][0], -5.49586, 1e-5) ||
    !vtkMathUtilities::FuzzyCompare(info.ComponentRanges[0][1], 5.79029, 1e-5))
  {
    std::cerr << "Unexpected coloring component range" << std::endl;
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(info.MagnitudeRange[0], 0., 1e-5) ||
    !vtkMathUtilities::FuzzyCompare(info.MagnitudeRange[1], 6.25568, 1e-5))
  {
    std::cerr << "Unexpected coloring magnitude range" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
