#include "vtkF3DGLTFDracoImporter.h"

#include <vtkMemoryResourceStream.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DGLTFDracoImporterStreamError(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  if (vtkF3DGLTFDracoImporter::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success\n";
    return EXIT_FAILURE;
  }


#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260212)
  vtkNew<vtkMemoryResourceStream> invalidStream;
  invalidStream->SetBuffer("invalid", 7);
  if (vtkF3DGLTFDracoImporter::CanReadFile(invalidStream))
  {
    std::cerr << "Unexpected CanReadFile success\n";
    return EXIT_FAILURE;
  }
#endif  

  // No need to actually use the reader, already covered by other tests
  return EXIT_SUCCESS;
}
