#include "vtkF3DAssimpImporter.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DAssimpImporterStreamError(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/animatedWorld.fbx";
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  // The loaded file has external texture, which requires a filename
  // this test the warning code path when texture cannot be loaded for this reason
  vtkNew<vtkF3DAssimpImporter> importer;
  importer->SetStream(stream);
  importer->SetMemoryHint("fbx");
  importer->Update();
  return EXIT_SUCCESS;
}
