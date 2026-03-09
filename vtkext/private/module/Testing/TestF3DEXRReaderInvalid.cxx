#include <vtkNew.h>

#include "vtkF3DEXRReader.h"

#include <iostream>

int TestF3DEXRReaderInvalid(int argc, char* argv[])
{
  vtkNew<vtkF3DEXRReader> reader;
  reader->Update();

  // Do not create a dummy.exr
  std::string filename = std::string(argv[1]) + "data/dummy.exr";
  reader->CanReadFile(filename.c_str());

  filename = std::string(argv[1]) + "data/invalid.exr";
  reader->SetFileName(filename.c_str());
  reader->Update();

  filename = std::string(argv[1]) + "data/grayscale.exr";
  reader->SetFileName(filename.c_str());
  reader->Update();

  filename = std::string(argv[1]) + "data/invalid_cut.exr";
  reader->SetFileName(filename.c_str());
  reader->Update();

  return EXIT_SUCCESS;
}
