#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAlembicReader.h"

#include <iostream>

int TestF3DAlembicReader(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.abc";
  vtkNew<vtkF3DAlembicReader> reader;
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(cout);
  return reader->GetOutput() ? EXIT_SUCCESS : EXIT_FAILURE;
}
