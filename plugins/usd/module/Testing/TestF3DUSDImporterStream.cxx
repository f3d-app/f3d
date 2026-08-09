#include "vtkF3DUSDImporter.h"

#include <vtkDataSet.h>
#include <vtkFileResourceStream.h>
#include <vtkMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DUSDImporterStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/primitives.usda";
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  std::string hint;
  if (!vtkF3DUSDImporter::CanReadFile(stream, hint))
  {
    std::cerr << "Unexpected CanReadFile failure\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetStream(stream);
  importer->Update();
  return importer->GetRenderer()->GetActors()->GetNumberOfItems() == 5 ? EXIT_SUCCESS
                                                                       : EXIT_FAILURE;
}
