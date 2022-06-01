#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DReaderInstantiator.h"
#include "vtkF3DMetaReader.h"

int TestF3DMetaReaderInvalid(int argc, char* argv[])
{
  F3DReaderInstantiator ReaderInstantiator;
  std::string dummyFilename = std::string(argv[1]) + "data/foo.dummy";
  vtkNew<vtkF3DMetaReader> reader;
  reader->SetFileNameAndCreateInternalReader(dummyFilename);
  if (reader->IsReaderValid())
  {
    std::cerr << "Reader unexpectedly can read a non-existent file" << std::endl;
    return EXIT_SUCCESS;
  }
  reader->Update();
  reader->Print(cout);
  if (reader->GetOutputInformation(0))
  {
    std::cerr << "Reader unexpectedly provide an output information for a non-existent file" <<
      std::endl; return EXIT_SUCCESS;
  }

  // This is a WILL_FAIL test
  return EXIT_FAILURE;
}
