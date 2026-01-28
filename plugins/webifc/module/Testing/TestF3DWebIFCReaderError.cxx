#include <vtkCallbackCommand.h>
#include <vtkExecutive.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>

int TestF3DWebIFCReaderError(int vtkNotUsed(argc), char* argv[])
{
  // Suppress VTK error output during this test
  vtkNew<vtkCallbackCommand> nullCallback;
  nullCallback->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

  // Test with no filename and no stream
  {
    vtkNew<vtkF3DWebIFCReader> reader;
    reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->Update();

    vtkPolyData* output = reader->GetOutput();
    if (output && output->GetNumberOfPoints() > 0)
    {
      std::cerr << "Expected no geometry with no input" << '\n';
      return EXIT_FAILURE;
    }
  }

  // Test with non-existent file
  {
    vtkNew<vtkF3DWebIFCReader> reader;
    reader->SetFileName("/non/existent/file.ifc");
    reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->Update();

    vtkPolyData* output = reader->GetOutput();
    if (output && output->GetNumberOfPoints() > 0)
    {
      std::cerr << "Expected no geometry from non-existent file" << '\n';
      return EXIT_FAILURE;
    }
  }

  // Test with invalid IFC file
  {
    std::string filename = std::string(argv[1]) + "data/ifc/invalid.ifc";

    vtkNew<vtkF3DWebIFCReader> reader;
    reader->SetFileName(filename);
    reader->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, nullCallback);
    reader->Update();

    vtkPolyData* output = reader->GetOutput();
    if (output && output->GetNumberOfPoints() > 0)
    {
      std::cerr << "Expected no geometry from invalid file" << '\n';
      return EXIT_FAILURE;
    }
  }

  std::cout << "Error handling test passed" << '\n';

  return EXIT_SUCCESS;
}
