#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include "vtkF3DPSDReader.h"

#include <iostream>

int TestF3DPSDReaderInvalid(int argc, char* argv[])
{
  vtkNew<vtkF3DPSDReader> reader;
  reader->Update();

  // Do not create a dummy.psd
  std::string filename = std::string(argv[1]) + "data/dummy.psd";
  reader->CanReadFile(filename.c_str());

  filename = std::string(argv[1]) + "data/invalid.psd";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

  filename = std::string(argv[1]) + "data/invalid_signature.psd";
  reader->SetFileName(filename.c_str());
  reader->CanReadFile(filename.c_str());
  reader->Update();

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  reader->CanReadFile(static_cast<vtkResourceStream*>(nullptr));

  const vtkNew<vtkMemoryResourceStream> emptyStream;
  reader->CanReadFile(emptyStream);

  vtkNew<vtkMemoryResourceStream> invalidStream;
  invalidStream->SetBuffer("9BPS", 4);
  reader->CanReadFile(invalidStream);
#endif

  return EXIT_SUCCESS;
}
