#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DSPZReader.h"

#include <iostream>

int TestF3DSPZReader(int vtkNotUsed(argc), char* argv[])
{
  std::string path = std::string(argv[1]) + "data/hornedlizard_small_d0.spz";

  vtkNew<vtkFileResourceStream> stream;

  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DSPZReader> reader;
  reader->SetStream(stream);
  reader->Update();

  vtkIdType nbGaussians = reader->GetOutput()->GetNumberOfPoints();

  if (nbGaussians != 13296)
  {
    std::cerr << "Incorrect number of gaussians: " << nbGaussians << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
