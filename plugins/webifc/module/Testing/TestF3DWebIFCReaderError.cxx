#include <vtkCallbackCommand.h>
#include <vtkExecutive.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>

int TestF3DWebIFCReaderError(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/ifc/invalid.ifc";

  // Suppress VTK error output during this test
  vtkNew<vtkCallbackCommand> nullCallback;
  nullCallback->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetFileName(filename);
  reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullCallback);

  reader->Update();

  vtkPolyData* output = reader->GetOutput();

  // With an invalid file, we expect empty geometry
  if (output && output->GetNumberOfPoints() > 0)
  {
    std::cerr << "Expected no geometry from invalid file" << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "Error handling test passed" << '\n';

  return EXIT_SUCCESS;
}
