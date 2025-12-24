#include <vtkImageData.h>
#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include "vtkF3DEXRReader.h"

#include <fstream>
#include <iostream>
#include <vector>

int TestF3DEXRMemReader(int argc, char* argv[])
{
  vtkNew<vtkF3DEXRReader> reader;
  reader->SetFileName("readFromMem.exr");

  // check failure
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251016)
  vtkNew<vtkMemoryResourceStream> nullStream;
  nullStream->SetBuffer(nullptr, 0);
  reader->SetStream(nullStream);
#else
  reader->SetMemoryBuffer("foo");
  reader->SetMemoryBufferLength(0);
#endif
  reader->Update();

  // read file to buffer
  std::ifstream file(std::string(argv[1]) + "data/Rec709.exr", std::ios::binary | std::ios::ate);
  if (!file)
  {
    return EXIT_FAILURE;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buff(size);
  if (!file.read(buff.data(), size))
  {
    return EXIT_FAILURE;
  }

  // set buffer and read
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20251016)
  vtkNew<vtkMemoryResourceStream> stream;
  stream->SetBuffer(buff.data(), buff.size());
  reader->SetStream(stream);
#else
  reader->SetMemoryBuffer(buff.data());
  reader->SetMemoryBufferLength(buff.size());
#endif
  reader->Update();

  reader->Print(std::cout);

  vtkImageData* img = reader->GetOutput();

  const int* dims = img->GetDimensions();

  if (dims[0] != 610 && dims[1] != 406)
  {
    std::cerr << "Incorrect EXR image size: " << dims[0] << "x" << dims[1] << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
