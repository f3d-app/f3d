#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DPLYReader.h"

#include <iostream>

int TestF3DPLYReader(int vtkNotUsed(argc), char* argv[])
{
  std::string path = std::string(argv[1]) + "data/bonsai_small.ply";

  {
    vtkNew<vtkFileResourceStream> stream;

    if (!stream->Open(path.c_str()))
    {
      std::cerr << "Cannot open file" << std::endl;
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DPLYReader> reader;
    reader->ReadFromInputStreamOn();
    reader->SetStream(stream);
    reader->Update();

    vtkIdType nbGaussians = reader->GetOutput()->GetNumberOfPoints();

    if (nbGaussians != 2655)
    {
      std::cerr << "Incorrect number of gaussians: " << nbGaussians << std::endl;
      return EXIT_FAILURE;
    }

    if (reader->GetOutput()->GetPointData()->GetArray("sh10") == nullptr)
    {
      std::cerr << "Cannot find spherical harmonics" << std::endl;
      return EXIT_FAILURE;
    }
  }

  {
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
      std::cerr << "Cannot open file" << std::endl;
      return EXIT_FAILURE;
    }

    std::string inputString(
      (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    vtkNew<vtkF3DPLYReader> reader;
    reader->ReadFromInputStringOn();
    reader->SetInputString(inputString);
    reader->Update();

    vtkIdType nbGaussians = reader->GetOutput()->GetNumberOfPoints();

    if (nbGaussians != 2655)
    {
      std::cerr << "Incorrect number of gaussians: " << nbGaussians << std::endl;
      return EXIT_FAILURE;
    }

    if (reader->GetOutput()->GetPointData()->GetArray("sh10") == nullptr)
    {
      std::cerr << "Cannot find spherical harmonics" << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
