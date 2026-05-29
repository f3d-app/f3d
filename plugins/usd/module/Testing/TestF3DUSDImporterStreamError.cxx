#include "vtkF3DUSDImporter.h"

#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include <iostream>

int TestF3DUSDImporterStreamError(int vtkNotUsed(argc), char* argv[])
{
  vtkObject::SetGlobalWarningDisplay(0);

  if (vtkF3DUSDImporter::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected success CanReadFile(nullptr)\n";
    return EXIT_FAILURE;
  }

  // The loaded file has external texture, which requires a filename
  // this test the warning code path when texture cannot be loaded for this reason
  {
    std::string filename = std::string(argv[1]) + "data/Teapot.usd";
    vtkNew<vtkFileResourceStream> stream;
    if (!stream->Open(filename.c_str()))
    {
      std::cerr << "Cannot open file\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DUSDImporter> importer;
    importer->SetStream(stream);

    // the update should succeed
    if (!importer->Update())
    {
      std::cerr << "Unexpected failure to update importer with valid USD stream\n";
      return EXIT_FAILURE;
    }
  }

  // Check that an error is emitted when trying to read a non-USD file from stream
  {
    std::string filename = std::string(argv[1]) + "data/f3d.glb";

    vtkNew<vtkFileResourceStream> stream;
    if (!stream->Open(filename.c_str()))
    {
      std::cerr << "Cannot open file\n";
      return EXIT_FAILURE;
    }

    if (vtkF3DUSDImporter::CanReadFile(stream))
    {
      std::cerr << "Unexpected success CanReadFile with glb stream\n";
      return EXIT_FAILURE;
    }

    vtkNew<vtkF3DUSDImporter> importer;
    importer->SetStream(stream);

    if (importer->Update())
    {
      std::cerr << "Unexpected success to update importer with invalid USD stream\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
