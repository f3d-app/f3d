#include "vtkF3DGLTFImporter.h"

#include <iostream>

int TestF3DGLTFImporter(int vtkNotUsed(argc), char* argv[])
{
  if (vtkF3DGLTFImporter::CanReadFile(nullptr))
  {
    std::cerr << "Unexpectedly can read a null file\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
