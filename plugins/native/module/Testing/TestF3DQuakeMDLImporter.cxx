#include <vtkDataSet.h>
#include <vtkFileResourceStream.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include "vtkF3DQuakeMDLImporter.h"

#include <iostream>

int TestF3DQuakeMDLImporter(int vtkNotUsed(argc), char* argv[])
{
  std::string path = std::string(argv[1]) + "data/zombie.mdl";

  vtkNew<vtkFileResourceStream> stream;

  if (!stream->Open(path.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DQuakeMDLImporter> importer;
  importer->SetStream(stream);
  importer->Update();
  return importer->GetRenderer()
           ->GetActors()
           ->GetLastActor()
           ->GetMapper()
           ->GetInput()
           ->GetNumberOfPoints() == 1878
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
