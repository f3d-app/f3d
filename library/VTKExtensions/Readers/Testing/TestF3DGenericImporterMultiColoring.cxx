#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include "vtkF3DGenericImporter.h"

#include <iostream>

int TestF3DGenericImporterMultiColoring(int argc, char* argv[])
{
  vtkNew<vtkF3DGenericImporter> importer;

  // Read a vts and a vtu with same array names
  // but different component names and array ranges
  vtkNew<vtkXMLUnstructuredGridReader> readerVTU;
  std::string filename = std::string(argv[1]) + "data/bluntfin_t.vtu";
  readerVTU->SetFileName(filename.c_str());
  importer->AddInternalReader("VTU", readerVTU);

  vtkNew<vtkXMLStructuredGridReader> readerVTS;
  filename = std::string(argv[1]) + "data/bluntfin.vts";
  readerVTS->SetFileName(filename.c_str());
  importer->AddInternalReader("VTS", readerVTS);

  if (!importer->CanReadFile())
  {
    std::cerr << "Importer unexpectedly can not read valid files" << std::endl;
    return EXIT_FAILURE;
  }

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

  vtkF3DGenericImporter::ColoringInfo info;
  importer->GetInfoForColoring(false, idx, info);
  if (info.Name != "Momentum")
  {
    std::cerr << "Unexpected coloring name" << std::endl;
    return EXIT_FAILURE;
  }
  if (info.Arrays.size() != 2)
  {
    std::cerr << "Unexpected number of arrays" << std::endl;
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
    !vtkMathUtilities::FuzzyCompare(info.ComponentRanges[0][1], 5.0455, 1e-5))
  {
    std::cerr << "Unexpected coloring component range" << std::endl;
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(info.MagnitudeRange[0], 0., 1e-5) ||
    !vtkMathUtilities::FuzzyCompare(info.MagnitudeRange[1], 5.71596, 1e-5))
  {
    std::cerr << "Unexpected coloring magnitude range" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
