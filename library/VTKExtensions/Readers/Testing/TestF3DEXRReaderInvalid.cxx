#include <vtkNew.h>

#include "vtkF3DEXRReader.h"

#include <iostream>

int TestF3DEXRReaderInvalid(int argc, char* argv[])
{
  vtkNew<vtkF3DEXRReader> reader;

  std::string filename = std::string(argv[1]) + "data/invalid.exr";
  reader->SetFileName(filename.c_str());
  reader->Update();

  return EXIT_SUCCESS;
}
