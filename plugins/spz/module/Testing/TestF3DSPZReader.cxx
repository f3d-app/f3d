#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DSPZReader.h"

#include <iostream>

int TestF3DSPZReader(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/xxx.spz";
  vtkNew<vtkF3DSPZReader> reader;
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(cout);
  return reader->GetOutput() ? EXIT_SUCCESS : EXIT_FAILURE;
}
