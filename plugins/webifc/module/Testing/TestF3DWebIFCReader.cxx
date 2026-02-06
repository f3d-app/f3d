#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>

int TestF3DWebIFCReader(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/IfcOpenHouse_IFC4.ifc";

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetFileName(filename);
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  if (!output)
  {
    std::cerr << "No output from reader\n";
    return EXIT_FAILURE;
  }

  constexpr vtkIdType expectedPoints = 3218;
  constexpr vtkIdType expectedCells = 1098;

  if (output->GetNumberOfPoints() != expectedPoints)
  {
    std::cerr << "Expected " << expectedPoints << " points but got " << output->GetNumberOfPoints()
              << "\n";
    return EXIT_FAILURE;
  }

  if (output->GetNumberOfCells() != expectedCells)
  {
    std::cerr << "Expected " << expectedCells << " cells but got " << output->GetNumberOfCells()
              << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
