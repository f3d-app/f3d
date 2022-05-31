#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "F3DReaderInstantiator.h"
#include "vtkF3DMetaReader.h"

int TestF3DMetaReader(int argc, char* argv[])
{
  F3DReaderInstantiator ReaderInstantiator;
  vtkNew<vtkF3DMetaReader> reader;
  
/*  std::string dummyFilename = std::string(argv[1]) + "data/foo.dummy";
  reader->SetFileName(dummyFilename);
  if (reader->IsReaderValid())
  {
    std::cerr << "Reader unexpectedly can read a non-existent file" << std::endl;
    return EXIT_FAILURE;
  }
  reader->Update();
  reader->Print(cout);
  if (reader->GetOutputInformation(0))
  {
    std::cerr << "Reader unexpectedly provide an output information for a non-existent file" << std::endl;
    return EXIT_FAILURE;
  }*/ // TODO
  
  std::string filename = std::string(argv[1]) + "data/cow.vtp";
  reader->SetFileName(filename);
  reader->SetFileName(filename);
  if (!reader->IsReaderValid())
  {
    std::cerr << "Reader unexpectedly can not read a valid file" << std::endl;
    return EXIT_FAILURE;
  }
  reader->Update();
  reader->Print(cout);
  if (!reader->GetOutputInformation(0))
  {
    std::cerr << "Reader unexpectedly dost not provide an output information for a valid file" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
