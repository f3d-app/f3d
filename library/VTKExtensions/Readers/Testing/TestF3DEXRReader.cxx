#include <vtkImageData.h>
#include <vtkNew.h>

#include "vtkF3DEXRReader.h"

#include <iostream>

int TestF3DEXRReader(int argc, char* argv[])
{
  vtkNew<vtkF3DEXRReader> reader;

  std::string filename = std::string(argv[1]) + "data/kloofendal_43d_clear_1k.exr";
  reader->SetFileName(filename.c_str());
  reader->Update();

  reader->Print(cout);

  vtkImageData* img = reader->GetOutput();

  int* dims = img->GetDimensions();

  if (dims[0] != 1024 && dims[1] != 512)
  {
    std::cerr << "Incorrect EXR image size." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
