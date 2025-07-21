#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkTestUtilities.h>
#include <vtkVersion.h>

#include "vtkF3DPLYReader.h"

#include <iostream>

int TestF3DPLYReader(int vtkNotUsed(argc), char* argv[])
{
  std::string pathGaussians = std::string(argv[1]) + "data/bonsai_small.ply";
  std::string pathSimplePoints = std::string(argv[1]) + "data/points.ply";

  // check open from stream
  {
    vtkNew<vtkFileResourceStream> stream;

    if (!stream->Open(pathGaussians.c_str()))
    {
      std::cerr << "Cannot open file\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DPLYReader> reader;
    reader->ReadFromInputStreamOn();
    reader->SetStream(stream);
    reader->Update();

    vtkIdType nbGaussians = reader->GetOutput()->GetNumberOfPoints();

    if (nbGaussians != 2655)
    {
      std::cerr << "Incorrect number of gaussians: " << nbGaussians << "\n";
      return EXIT_FAILURE;
    }

    if (reader->GetOutput()->GetPointData()->GetArray("sh10") == nullptr)
    {
      std::cerr << "Cannot find spherical harmonics\n";
      return EXIT_FAILURE;
    }
  }

  // check open from string
  {
    std::ifstream file;
    file.open(pathGaussians.c_str(), std::ios::binary);
    if (!file.is_open())
    {
      std::cerr << "Cannot open file\n";
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
      std::cerr << "Incorrect number of gaussians: " << nbGaussians << "\n";
      return EXIT_FAILURE;
    }

    if (reader->GetOutput()->GetPointData()->GetArray("sh10") == nullptr)
    {
      std::cerr << "Cannot find spherical harmonics\n";
      return EXIT_FAILURE;
    }
  }

  // check not 3d gaussians
  {
    vtkNew<vtkF3DPLYReader> reader;
    reader->SetFileName(pathSimplePoints.c_str());
    reader->Update();

    vtkIdType nbPoints = reader->GetOutput()->GetNumberOfPoints();

    if (nbPoints != 5)
    {
      std::cerr << "Incorrect number of points: " << nbPoints << "\n";
      return EXIT_FAILURE;
    }

    if (reader->GetOutput()->GetPointData()->GetArray("sh10") != nullptr)
    {
      std::cerr << "Should not have spherical harmonics\n";
      return EXIT_FAILURE;
    }
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20250703) // a leak was fixed in this version
  // check invalid
  {
    std::string pathInvalid = std::string(argv[1]) + "data/invalid.so";

    vtkNew<vtkF3DPLYReader> reader;
    reader->SetFileName(pathInvalid.c_str());
    reader->Update();

    vtkIdType nbPoints = reader->GetOutput()->GetNumberOfPoints();

    if (nbPoints != 0)
    {
      std::cerr << "The file should be invalid\n";
      return EXIT_FAILURE;
    }
  }
#endif

  return EXIT_SUCCESS;
}
