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
//  std::string filename = std::string(argv[1]) + "data/small.usdz";              // KO -> CANNOT PARSE PK
  std::string filename = std::string(argv[1]) + "data/suzanne.usd";             // KO -> CANNOT PARSE PXR-USDC
//  std::string filename = std::string(argv[1]) + "data/Teapot.usd";              // KO -> 4 assets missing
//  std::string filename = std::string(argv[1]) + "data/invalid.usd";             // KO -> expected as this is invalid
//  std::string filename = std::string(argv[1]) + "data/glyphs.usda";             // OK
//  std::string filename = std::string(argv[1]) + "data/two_quads_interp.usda";   // KO -> 1 asset missing
//  std::string filename = std::string(argv[1]) + "data/instancing.usda";         // KO -> primitives.usda
//  std::string filename = std::string(argv[1]) + "data/primitives.usda";         // OK
//  std::string filename = std::string(argv[1]) + "data/primitivesZ.usda";        // OK
  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open(filename.c_str()))
  {
    std::cerr << "Cannot open file\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DUSDImporter> importer;
  importer->SetStream(stream);
  importer->Update();
  return importer->GetRenderer()
           ->GetActors()
           ->GetLastActor()
           ->GetMapper()
           ->GetInput()
           ->GetNumberOfPoints() > 0
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
