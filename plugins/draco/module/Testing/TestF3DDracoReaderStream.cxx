#include "vtkF3DDracoReader.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DDracoReaderStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/suzanne.drc";
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (!vtkF3DDracoReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile failure\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DDracoReader> reader;
  reader->SetStream(stream);
  reader->Update();
  return reader->GetOutput()->GetNumberOfPoints() == 7958 ? EXIT_SUCCESS : EXIT_FAILURE;
}
