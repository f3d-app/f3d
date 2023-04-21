#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DDracoReader.h"

#include <iostream>

int TestF3DDracoReader(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.drc";
  vtkNew<vtkF3DDracoReader> reader;
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(cout);
  return EXIT_SUCCESS;
}
