#include "vtkF3DDracoReader.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DDracoReaderStreamError(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/invalid.drc";
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DDracoReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  // invalid.drc should be less than 5 chars long
  if (vtkF3DDracoReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success\n";
    return EXIT_FAILURE;
  }

  // No need to actually use the reader, already covered by other tests
  return EXIT_SUCCESS;
}
