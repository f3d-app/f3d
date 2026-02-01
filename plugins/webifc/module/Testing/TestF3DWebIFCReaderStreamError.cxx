#include <vtkCallbackCommand.h>
#include <vtkExecutive.h>
#include <vtkFileResourceStream.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>

int TestF3DWebIFCReaderStreamError(int vtkNotUsed(argc), char* argv[])
{
  std::string filename = std::string(argv[1]) + "data/invalid.ifc";

  vtkNew<vtkFileResourceStream> fileStream;
  if (!fileStream->Open(filename.c_str()))
  {
    std::cerr << "Failed to open file stream\n";
    return EXIT_FAILURE;
  }

  // Suppress VTK error output during this test
  vtkNew<vtkCallbackCommand> nullCallback;
  nullCallback->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetStream(fileStream);
  reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
  reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullCallback);

  reader->Update();

  vtkPolyData* output = reader->GetOutput();

  if (!output || output->GetNumberOfPoints() != 0)
  {
    std::cerr << "Expected output with 0 points from invalid stream\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
