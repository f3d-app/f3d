#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

int TestF3DOCCTReader(int argc, char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/cube.stp";
  vtkNew<vtkF3DOCCTReader> reader;
  reader->RelativeDeflectionOn();
  reader->SetLinearDeflection(0.1);
  reader->SetAngularDeflection(0.5);
  reader->ReadWireOn();
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(cout);
  return reader->GetOutput()->GetNumberOfBlocks() > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
