#include <vtkCellData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DIFCReader.h"

#include <iostream>
#include <sstream>

int TestF3DIFCReaderOptions(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/ifc/IfcOpenHouse.ifc";

  vtkNew<vtkF3DIFCReader> reader;
  reader->SetFileName(filename);

  // Test CircleSegments option
  reader->SetCircleSegments(24);
  if (reader->GetCircleSegments() != 24)
  {
    std::cerr << "CircleSegments getter/setter failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test ReadOpenings option
  reader->SetReadOpenings(true);
  if (!reader->GetReadOpenings())
  {
    std::cerr << "ReadOpenings getter/setter failed" << std::endl;
    return EXIT_FAILURE;
  }
  reader->ReadOpeningsOff();
  if (reader->GetReadOpenings())
  {
    std::cerr << "ReadOpeningsOff failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test ReadSpaces option
  reader->SetReadSpaces(true);
  if (!reader->GetReadSpaces())
  {
    std::cerr << "ReadSpaces getter/setter failed" << std::endl;
    return EXIT_FAILURE;
  }
  reader->ReadSpacesOff();
  if (reader->GetReadSpaces())
  {
    std::cerr << "ReadSpacesOff failed" << std::endl;
    return EXIT_FAILURE;
  }

  // Test PrintSelf
  std::ostringstream oss;
  reader->Print(oss);
  std::string printOutput = oss.str();
  if (printOutput.find("FileName") == std::string::npos ||
      printOutput.find("CircleSegments") == std::string::npos)
  {
    std::cerr << "PrintSelf output incomplete" << std::endl;
    return EXIT_FAILURE;
  }

  // Update and check output arrays
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  if (!output || output->GetNumberOfPoints() == 0)
  {
    std::cerr << "No output from reader" << std::endl;
    return EXIT_FAILURE;
  }

  // Check for Normals array
  if (!output->GetPointData()->GetNormals())
  {
    std::cerr << "PolyData has no normals" << std::endl;
    return EXIT_FAILURE;
  }

  // Check for Colors array
  if (!output->GetCellData()->GetScalars())
  {
    std::cerr << "PolyData has no colors" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "All options tests passed" << std::endl;

  return EXIT_SUCCESS;
}
