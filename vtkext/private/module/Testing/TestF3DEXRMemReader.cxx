#include <vtkImageData.h>
#include <vtkNew.h>

#include "vtkF3DEXRReader.h"

#include <fstream>
#include <iostream>
#include <vector>

bool readFileToVector(const std::string& filename, std::vector<char>& buffer)
{
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file)
  {
    return false;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  buffer.resize(size);
  return file.read(buffer.data(), size) ? true : false;
}

int TestF3DEXRMemReader(int argc, char* argv[])
{
  vtkNew<vtkF3DEXRReader> reader;

  std::string actual_filename = std::string(argv[1]) + "data/Rec709.exr";
  std::string filename = "readFromMem.exr";
  reader->SetFileName(filename.c_str());
  std::vector<char> buff;
  readFileToVector(actual_filename, buff);
  reader->SetMemoryBuffer(buff.data());
  reader->SetMemoryBufferLength(buff.size());
  reader->Update();

  reader->Print(cout);

  vtkImageData* img = reader->GetOutput();

  int* dims = img->GetDimensions();

  if (dims[0] != 610 && dims[1] != 406)
  {
    std::cerr << "Incorrect EXR image size." << dims[0] << ":" << dims[1] << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
