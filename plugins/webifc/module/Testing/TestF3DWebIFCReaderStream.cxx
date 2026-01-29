#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

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
  if (!output || output->GetNumberOfPoints() == 0)
  {
    std::cerr << "No output from stream reader\n";
    return EXIT_FAILURE;
  }

  std::cout << "Successfully read IFC stream with " << output->GetNumberOfPoints() << " points\n";

  return EXIT_SUCCESS;
}
