#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <cassert>
#include <iostream>

void testReader(const std::string& filename, const vtkF3DOCCTReader::FILE_FORMAT& format)
{
  vtkNew<vtkF3DOCCTReader> reader;
  reader->RelativeDeflectionOn();
  reader->SetLinearDeflection(0.1);
  reader->SetAngularDeflection(0.5);
  reader->ReadWireOn();
  reader->SetFileName(filename);
  reader->SetFileFormat(format);
  reader->Update();
  reader->Print(cout);
  assert(reader->GetOutput()->GetNumberOfBlocks() > 0);
}

int TestF3DOCCTReader(int vtkNotUsed(argc), char* argv[])
{
  testReader(std::string(argv[1]) + "data/f3d.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP);
  testReader(std::string(argv[1]) + "data/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES);
  testReader(std::string(argv[1]) + "data/f3d.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP);
  return EXIT_SUCCESS;
}
