#include <vtkImageData.h>
#include <vtkNew.h>

#include "vtkF3DWebPReader.h"

#include <fstream>
#include <iostream>
#include <vector>

int TestF3DWebPMemReader(int argc, char* argv[])
{
  vtkNew<vtkF3DWebPReader> reader;
  reader->SetFileName("image.webp");

  // check failure
  reader->SetMemoryBuffer("foo");
  reader->SetMemoryBufferLength(0);
  reader->Update();

  // read file to buffer
  std::ifstream file(std::string(argv[1]) + "data/image.webp", std::ios::binary | std::ios::ate);
  if (!file)
  {
    std::cerr << "Cannot open the file\n";
    return EXIT_FAILURE;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buff(size);
  if (!file.read(buff.data(), size))
  {
    std::cerr << "Cannot read the file\n";
    return EXIT_FAILURE;
  }

  // set buffer and read
  reader->SetMemoryBuffer(buff.data());
  reader->SetMemoryBufferLength(buff.size());
  reader->Update();

  reader->Print(cout);

  vtkImageData* img = reader->GetOutput();

  const int* dims = img->GetDimensions();

  if (dims[0] != 1024 && dims[1] != 1024)
  {
    std::cerr << "Incorrect WebP image size." << dims[0] << ":" << dims[1] << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
