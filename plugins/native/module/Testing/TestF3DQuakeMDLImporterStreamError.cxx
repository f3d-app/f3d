#include "vtkF3DQuakeMDLImporter.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DQuakeMDLImporterStreamError(int vtkNotUsed(argc), char* argv[])
{
  if (vtkF3DQuakeMDLImporter::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkFileResourceStream> stream;
  std::string path = std::string(argv[1]) + "data/invalid.mdl";
  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DQuakeMDLImporter::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success invalid file\n";
    return EXIT_FAILURE;
  }

  path = std::string(argv[1]) + "data/invalid_version.mdl";
  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DQuakeMDLImporter::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success wrong version file\n";
    return EXIT_FAILURE;
  }

  // No need to read invalid file, already covered
  return EXIT_SUCCESS;
}
