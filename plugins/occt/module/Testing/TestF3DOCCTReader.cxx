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
  const std::string data = std::string(argv[1]) + "data";
  return (testReader(data + "/f3d.stp", vtkF3DOCCTReader::FILE_FORMAT::STEP) &&
           testReader(data + "/f3d.igs", vtkF3DOCCTReader::FILE_FORMAT::IGES) &&
           testReader(data + "/f3d.brep", vtkF3DOCCTReader::FILE_FORMAT::BREP) &&
           testReader(data + "/f3d.xbf", vtkF3DOCCTReader::FILE_FORMAT::XBF))
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
