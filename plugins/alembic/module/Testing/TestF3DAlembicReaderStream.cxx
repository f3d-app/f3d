#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DAlembicReader.h"

#include <iostream>

int TestF3DAlembicReaderStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.abc";

  vtkNew<vtkFileResourceStream> fileStream;
  fileStream->Open(filename.c_str());

  if (!vtkF3DAlembicReader::CanReadFile(fileStream))
  {
    std::cerr << "Unexpected CanReadFile result\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DAlembicReader> reader;
  reader->SetStream(fileStream);
  reader->Update();
  return reader->GetOutput()->GetNumberOfPoints() > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
