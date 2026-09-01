#include <vtkFileResourceStream.h>
#include <vtkNew.h>

#include "vtkF3DLYSReader.h"

#include <iostream>

int TestF3DLYSReader(int vtkNotUsed(argc), char* argv[])
{
  if (vtkF3DLYSReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  std::string path = std::string(argv[1]) + "data/bunny.lys";

  // Test reading from stream
  {
    vtkNew<vtkFileResourceStream> stream;
    if (!stream->Open(path.c_str()))
    {
      std::cerr << "Cannot open file: " << path << "\n";
      return EXIT_FAILURE;
    }

    if (!vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile failure on bunny.lys\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DLYSReader> reader;
    reader->SetStream(stream);
    reader->Update();

    vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
    if (nPolys != 3851)
    {
      std::cerr << "Incorrect number of polygons: " << nPolys << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test reading from file name
  {
    vtkNew<vtkF3DLYSReader> reader;
    reader->SetFileName(path.c_str());
    reader->Update();

    vtkIdType nPoints = reader->GetOutput()->GetNumberOfPoints();
    if (nPoints != 1887)
    {
      std::cerr << "Incorrect number of points with SetFileName: " << nPoints << "\n";
      return EXIT_FAILURE;
    }

    vtkIdType nPolys = reader->GetOutput()->GetNumberOfPolys();
    if (nPolys != 3851)
    {
      std::cerr << "Incorrect number of polygons with SetFileName: " << nPolys << "\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with non-LYS valid file
  {
    std::string pathVtp = std::string(argv[1]) + "data/f3d.vtp";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathVtp.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on f3d.vtp\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with empty file
  {
    std::string pathEmpty = std::string(argv[1]) + "data/empty.splat";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathEmpty.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on empty file\n";
      return EXIT_FAILURE;
    }
  }

  // Test CanReadFile with invalid/corrupted file
  {
    std::string pathInvalid = std::string(argv[1]) + "data/invalid.splat";
    vtkNew<vtkFileResourceStream> stream;
    if (stream->Open(pathInvalid.c_str()) && vtkF3DLYSReader::CanReadFile(stream))
    {
      std::cerr << "Unexpected CanReadFile success on invalid file\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
