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

  vtkNew<vtkFileResourceStream> stream;

  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (!vtkF3DLYSReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile failure\n";
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

  path = std::string(argv[1]) + "data/f3d.vtp";
  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DLYSReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
