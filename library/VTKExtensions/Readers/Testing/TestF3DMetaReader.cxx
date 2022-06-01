#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DReaderInstantiator.h"
#include "vtkF3DMetaReader.h"

int TestF3DMetaReader(int argc, char* argv[])
{
  F3DReaderInstantiator ReaderInstantiator;
  std::string filename = std::string(argv[1]) + "data/cow.vtp";
  vtkNew<vtkF3DMetaReader> reader;
  reader->SetFileNameAndCreateInternalReader(filename);
  reader->SetFileNameAndCreateInternalReader(filename); // Double call for the MTime mechanism coverage
  if (!reader->IsReaderValid())
  {
    std::cerr << "Reader unexpectedly can not read a valid file" << std::endl;
    return EXIT_FAILURE;
  }
  reader->Update();
  reader->Print(cout);
  if (!reader->GetOutputInformation(0))
  {
    std::cerr << "Reader unexpectedly dost not provide an output information for a valid file"
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
