#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>

#include "vtkF3DWebIFCReader.h"

#include <iostream>

int TestF3DWebIFCReader(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <testing_dir>" << '\n';
    return EXIT_FAILURE;
  }

  std::string testingDir = argv[1];
  std::string filename = testingDir + "data/ifc/IfcOpenHouse_IFC4.ifc";

  vtkNew<vtkF3DWebIFCReader> reader;
  reader->SetFileName(filename);
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  if (!output)
  {
    std::cerr << "No output from reader" << '\n';
    return EXIT_FAILURE;
  }

  if (output->GetNumberOfPoints() == 0)
  {
    std::cerr << "PolyData has no points" << '\n';
    return EXIT_FAILURE;
  }

  if (output->GetNumberOfCells() == 0)
  {
    std::cerr << "PolyData has no cells" << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "Successfully read IFC file with " << output->GetNumberOfPoints() << " points and "
            << output->GetNumberOfCells() << " cells" << '\n';

  return EXIT_SUCCESS;
}
