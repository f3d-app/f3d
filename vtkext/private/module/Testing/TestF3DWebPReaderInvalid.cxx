#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include "vtkF3DWebPReader.h"

#include <iostream>

int TestF3DWebPReaderInvalid(int argc, char* argv[])
{
  vtkNew<vtkF3DWebPReader> reader;
  reader->Update();

  // Do not create a dummy.webp
  std::string filename = std::string(argv[1]) + "data/dummy.webp";
  reader->CanReadFile(filename.c_str());

  filename = std::string(argv[1]) + "data/invalid.webp";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

  filename = std::string(argv[1]) + "data/invalid2.webp";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  reader->CanReadFile(static_cast<vtkResourceStream*>(nullptr));

  vtkNew<vtkMemoryResourceStream> emptyStream;
  reader->CanReadFile(emptyStream);

  vtkNew<vtkMemoryResourceStream> invalidStream;
  invalidStream->SetBuffer("RIFFxxxxNOPE", 12);
  reader->CanReadFile(invalidStream);
#endif

  return EXIT_SUCCESS;
}
