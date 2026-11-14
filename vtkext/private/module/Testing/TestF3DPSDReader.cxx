#include <vtkImageData.h>
#include <vtkNew.h>

#include "vtkF3DPSDReader.h"

#include <iostream>

int TestF3DPSDReader(int argc, char* argv[])
{
  vtkNew<vtkF3DPSDReader> reader;

  const std::string filename = std::string(argv[1]) + "data/image.psd";
  if (reader->CanReadFile(filename.c_str()) == 0)
  {
    std::cerr << "Unexpected CanReadFile failure.\n";
    return EXIT_FAILURE;
  }

  reader->SetFileName(filename.c_str());
  reader->Update();

  std::cout << "Reader Name: " << reader->GetDescriptiveName() << '\n';

  vtkImageData* img = reader->GetOutput();

  const int* dims = img->GetDimensions();

  if (dims[0] != 640 && dims[1] != 426)
  {
    std::cerr << "Incorrect PSD image size.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
