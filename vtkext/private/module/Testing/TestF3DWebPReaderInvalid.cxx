#include <vtkNew.h>

#include "vtkF3DWebPReader.h"

#include <iostream>

int TestF3DWebPReaderInvalid(int argc, char* argv[])
{
  vtkNew<vtkF3DWebPReader> reader;
  reader->Update();

  // Do not create a dummy.webp
  std::string filename = std::string(argv[1]) + "data/dummy.webp";
  reader->CanReadFile(filename.c_str());

  filename = std::string(argv[1]) + "data/invalid.webp";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

  filename = std::string(argv[1]) + "data/invalid2.webp";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

  return EXIT_SUCCESS;
}
