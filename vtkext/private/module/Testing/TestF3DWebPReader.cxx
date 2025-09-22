#include <vtkImageData.h>
#include <vtkNew.h>

#include "vtkF3DWebPReader.h"

#include <iostream>

int TestF3DWebPReader(int argc, char* argv[])
{
  vtkNew<vtkF3DWebPReader> reader;

  std::string filename = std::string(argv[1]) + "data/image.webp";
  reader->SetFileName(filename.c_str());
  reader->Update();

  std::cout << "Reader Name: " << reader->GetDescriptiveName() << '\n';

  vtkImageData* img = reader->GetOutput();

  const int* dims = img->GetDimensions();

  if (dims[0] != 1024 && dims[1] != 1024)
  {
    std::cerr << "Incorrect WebP image size.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
