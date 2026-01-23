#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DIFCReader.h"

#include <iostream>

int TestF3DIFCReaderStream(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/ifc/IfcOpenHouse.ifc";

  vtkNew<vtkFileResourceStream> fileStream;
  if (!fileStream->Open(filename.c_str()))
  {
    std::cerr << "Failed to open file stream" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DIFCReader> reader;
  reader->SetStream(fileStream);
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  if (!output || output->GetNumberOfPoints() == 0)
  {
    std::cerr << "No output from stream reader" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Successfully read IFC stream with " << output->GetNumberOfPoints() << " points"
            << std::endl;

  return EXIT_SUCCESS;
}
