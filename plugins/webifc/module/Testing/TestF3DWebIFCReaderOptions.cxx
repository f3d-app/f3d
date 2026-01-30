#include <vtkCellData.h>
#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>
#include <sstream>

int TestF3DWebIFCReaderOptions(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/IfcOpenHouse_IFC4.ifc";

  // Test CanReadFile with valid IFC file
  vtkNew<vtkFileResourceStream> validStream;
  if (!validStream->Open(filename.c_str()))
  {
    std::cerr << "Failed to open valid IFC file\n";
    return EXIT_FAILURE;
  }
  if (!vtkF3DWebIFCReader::CanReadFile(validStream))
  {
    std::cerr << "CanReadFile returned false for valid IFC file\n";
    return EXIT_FAILURE;
  }

  // Test CanReadFile with invalid file
  std::string invalidFilename = std::string(argv[1]) + "data/cow.vtp";
  vtkNew<vtkFileResourceStream> invalidStream;
  if (invalidStream->Open(invalidFilename.c_str()))
  {
    if (vtkF3DWebIFCReader::CanReadFile(invalidStream))
    {
      std::cerr << "CanReadFile returned true for non-IFC file\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with STEP file (same ISO-10303-21 header but different FILE_SCHEMA)
  std::string stepFilename = std::string(argv[1]) + "data/f3d.stp";
  vtkNew<vtkFileResourceStream> stepStream;
  if (stepStream->Open(stepFilename.c_str()))
  {
    if (vtkF3DWebIFCReader::CanReadFile(stepStream))
    {
      std::cerr << "CanReadFile returned true for STEP file\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with null stream
  if (vtkF3DWebIFCReader::CanReadFile(nullptr))
  {
    std::cerr << "CanReadFile returned true for null stream\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetFileName(filename);

  // Test GetFileName
  if (reader->GetFileName() != filename)
  {
    std::cerr << "GetFileName failed\n";
    return EXIT_FAILURE;
  }

  // Test GetStream (should be null initially)
  if (reader->GetStream() != nullptr)
  {
    std::cerr << "GetStream should be null initially\n";
    return EXIT_FAILURE;
  }

  // Test CircleSegments option
  reader->SetCircleSegments(24);
  if (reader->GetCircleSegments() != 24)
  {
    std::cerr << "CircleSegments getter/setter failed\n";
    return EXIT_FAILURE;
  }

  // Test ReadOpenings option
  reader->SetReadOpenings(true);
  if (!reader->GetReadOpenings())
  {
    std::cerr << "ReadOpenings getter/setter failed\n";
    return EXIT_FAILURE;
  }
  reader->ReadOpeningsOff();
  if (reader->GetReadOpenings())
  {
    std::cerr << "ReadOpeningsOff failed\n";
    return EXIT_FAILURE;
  }

  // Test ReadSpaces option
  reader->SetReadSpaces(true);
  if (!reader->GetReadSpaces())
  {
    std::cerr << "ReadSpaces getter/setter failed\n";
    return EXIT_FAILURE;
  }
  reader->ReadSpacesOff();
  if (reader->GetReadSpaces())
  {
    std::cerr << "ReadSpacesOff failed\n";
    return EXIT_FAILURE;
  }

  // Test PrintSelf
  std::ostringstream oss;
  reader->Print(oss);
  std::string printOutput = oss.str();
  if (printOutput.find("FileName") == std::string::npos ||
    printOutput.find("CircleSegments") == std::string::npos)
  {
    std::cerr << "PrintSelf output incomplete\n";
    return EXIT_FAILURE;
  }

  // Update and check output arrays
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  if (!output || output->GetNumberOfPoints() == 0)
  {
    std::cerr << "No output from reader\n";
    return EXIT_FAILURE;
  }

  // Check for Normals array
  if (!output->GetPointData()->GetNormals())
  {
    std::cerr << "PolyData has no normals\n";
    return EXIT_FAILURE;
  }

  // Check for Colors array
  if (!output->GetCellData()->GetScalars())
  {
    std::cerr << "PolyData has no colors\n";
    return EXIT_FAILURE;
  }

  std::cout << "All options tests passed\n";

  return EXIT_SUCCESS;
}
