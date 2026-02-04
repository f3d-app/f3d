#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DSplatReader.h"

#include <iostream>

int TestF3DSplatReader(int vtkNotUsed(argc), char* argv[])
{
  std::string path = std::string(argv[1]) + "data/small.splat";

  vtkNew<vtkFileResourceStream> stream;

  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open .splat file\n";
    return EXIT_FAILURE;
  }

  if (!vtkF3DSplatReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile failure\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DSplatReader> reader;
  reader->SetStream(stream);
  reader->Update();

  vtkIdType nbPoints = reader->GetOutput()->GetNumberOfPoints();

  if (nbPoints != 52293) // <-- Change this to expected number of splats
  {
    std::cerr << "Incorrect number of splats: " << nbPoints << "\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DSplatReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  path = std::string(argv[1]) + "data/invalid.splat";
  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open invalid.splat file\n";
    return EXIT_FAILURE;
  }
  if (vtkF3DSplatReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
