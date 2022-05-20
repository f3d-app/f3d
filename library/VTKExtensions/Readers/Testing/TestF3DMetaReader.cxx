#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DReaderInstantiator.h"
#include "vtkF3DMetaReader.h"

int TestF3DMetaReader(int argc, char* argv[])
{
  // TODO actual test

  F3DReaderInstantiator ReaderInstantiator;
  std::string filename = std::string(argv[1]) + "data/cow.vtp";
  vtkNew<vtkF3DMetaReader> reader;
  reader->SetFileName(std::string(filename.c_str()));
  reader->Update();
  return EXIT_SUCCESS;
}
