#include "vtkF3DGenericImporter.h"
#include "vtkF3DMetaImporter.h"

#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <iostream>

int TestF3DMetaImporterMultiColoring(int argc, char* argv[])
{
  vtkNew<vtkF3DMetaImporter> importer;

  // Check importer error code path
  if (!importer->GetCameraName(0).empty())
  {
    std::cerr << "Unexpected camera name that should be empty\n";
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

  // Test coloring handler
  F3DColoringInfoHandler& coloringHandler = importer->GetColoringInfoHandler();

  auto info = coloringHandler.SetCurrentColoring(true, false, "Momentum", false);
  if (!info.has_value())
  {
    std::cerr << "Coloring handler unable to set coloring as expected\n";
    return EXIT_FAILURE;
  }
  if (info.value().Name != "Momentum")
  {
    std::cerr << "Unexpected coloring name: " << info.value().Name << "\n";
    return EXIT_FAILURE;
  }
  if (info.value().MaximumNumberOfComponents != 3)
  {
    std::cerr << "Unexpected coloring nComp\n";
    return EXIT_FAILURE;
  }
  if (info.value().ComponentNames[0] != "LX Momentum")
  {
    std::cerr << "Unexpected coloring component name 0\n";
    return EXIT_FAILURE;
  }
  if (info.value().ComponentNames[1] != "LY Momentum_t")
  {
    std::cerr << "Unexpected coloring component name 1\n";
    return EXIT_FAILURE;
  }
  if (info.value().ComponentNames[2] != "")
  {
    std::cerr << "Unexpected coloring component name 2\n";
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(info.value().ComponentRanges[0][0], -5.49586, 1e-5) ||
    !vtkMathUtilities::FuzzyCompare(info.value().ComponentRanges[0][1], 5.79029, 1e-5))
  {
    std::cerr << "Unexpected coloring component range\n";
    return EXIT_FAILURE;
  }
  if (!vtkMathUtilities::FuzzyCompare(info.value().MagnitudeRange[0], 0., 1e-5) ||
    !vtkMathUtilities::FuzzyCompare(info.value().MagnitudeRange[1], 6.25568, 1e-5))
  {
    std::cerr << "Unexpected coloring magnitude range\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
