#include "vtkF3DAssimpImporter.h"

#include <vtkDataSet.h>
#include <vtkFileResourceStream.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DAssimpImporterStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/texturedCube.fbx";
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  std::string hint;
  if (!vtkF3DAssimpImporter::CanReadFile(stream, hint))
  {
    std::cerr << "Unexpected CanReadFile failure\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DAssimpImporter> importer;
  importer->SetStream(stream);
  importer->SetMemoryHint("fbx");
  importer->Update();
  return importer->GetRenderer()
           ->GetActors()
           ->GetLastActor()
           ->GetMapper()
           ->GetInput()
           ->GetNumberOfPoints() == 24
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
