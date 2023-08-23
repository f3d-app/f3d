#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

bool testReader(const std::string& filename, const vtkF3DOCCTReader::FILE_FORMAT& format)
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
  return reader->GetOutput()->GetNumberOfBlocks() > 0;
}

int TestF3DOCCTReader(int vtkNotUsed(argc), char* argv[])
{
  return (testReader(std::string(argv[1]) + "data/f3d.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP) &&
           testReader(std::string(argv[1]) + "data/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES) &&
           testReader(std::string(argv[1]) + "data/f3d.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP))
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
