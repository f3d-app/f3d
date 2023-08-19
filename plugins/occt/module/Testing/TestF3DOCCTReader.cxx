#include <cassert>

#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOCCTReader.h"

#include <iostream>

int TestF3DOCCTReader(int vtkNotUsed(argc), char* argv[])
{
  for (const auto file : {
         "data/f3d.stp",
         "data/f3d.igs",
         "data/f3d.brep",
       })
  {
    std::string filename = std::string(argv[1]) + file;
    vtkNew<vtkF3DOCCTReader> reader;
    reader->RelativeDeflectionOn();
    reader->SetLinearDeflection(0.1);
    reader->SetAngularDeflection(0.5);
    reader->ReadWireOn();
    reader->SetFileName(filename);
    reader->Update();
    reader->Print(cout);
    assert(reader->GetOutput()->GetNumberOfBlocks() > 0);
  }

  return EXIT_SUCCESS;
}
