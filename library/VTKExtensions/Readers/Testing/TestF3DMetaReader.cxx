#include <vtkNew.h>
#include <vtkTestUtilities.h>
#include <vtkXMLPolyDataReader.h>

#include "vtkF3DMetaReader.h"

int TestF3DMetaReader(int argc, char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/cow.vtp";
  vtkNew<vtkXMLPolyDataReader> internalReader;
  internalReader->SetFileName(filename.c_str());
  vtkNew<vtkF3DMetaReader> reader;
  reader->SetFileName(filename);
  reader->SetInternalReader(internalReader);
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
