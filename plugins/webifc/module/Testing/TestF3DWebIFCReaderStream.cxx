#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <cassert>
#include <iostream>

int TestF3DWebIFCReaderStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/IfcOpenHouse_IFC4.ifc";

  vtkNew<vtkFileResourceStream> fileStream;
  if (!fileStream->Open(filename.c_str()))
  {
    std::cerr << "Failed to open file stream\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetStream(fileStream);
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  assert(output);

  constexpr vtkIdType expectedPoints = 3218;
  if (output->GetNumberOfPoints() != expectedPoints)
  {
    std::cerr << "Expected " << expectedPoints << " points but got "
              << output->GetNumberOfPoints() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
