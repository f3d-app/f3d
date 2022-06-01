#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

int TestF3DOCCTReader(int argc, char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/cube.stp";
  vtkNew<vtkF3DOCCTReader> reader;
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(cout);
  return reader->GetOutput()->GetNumberOfPoints() > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
