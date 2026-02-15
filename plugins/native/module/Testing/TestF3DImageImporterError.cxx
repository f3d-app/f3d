#include "vtkF3DImageImporter.h"

#include <vtkCallbackCommand.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DImageImporterError(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Suppress VTK error output during this test
  vtkNew<vtkCallbackCommand> nullCallback;
  nullCallback->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

  vtkNew<vtkF3DImageImporter> importer;
  importer->AddObserver(vtkCommand::ErrorEvent, nullCallback);
  if (importer->Update())
  {
    std::cerr << "Unexpected Update success with no ImageHint\n";
    return EXIT_FAILURE;
  }

  importer->SetImageHint("invalid");
  if (importer->Update())
  {
    std::cerr << "Unexpected Update success with invalid ImageHint\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
