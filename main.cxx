#include <cstdio>
#include <iostream>

#include "cxxopts.hpp"

#include "vtkF3DInteractor.h"
#include "vtkF3DGenericImporter.h"

#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

int main(int argc, char **argv)
{
  try
  {
    cxxopts::Options options(argv[0], " - f3d viewer command line options");
    options
      .positional_help("[optional args]")
      .show_positional_help();

    bool normals = false;

    options
      .allow_unrecognised_options()
      .add_options()
      ("n,normals", "Show mesh normals", cxxopts::value<bool>(normals))
      ("i,input", "Input", cxxopts::value<std::string>())
      ("positional",
        "Positional arguments: these are the arguments that are entered "
        "without an option", cxxopts::value<std::vector<std::string>>())
      ("help", "Print help")
    ;

    options.parse_positional({"input", "positional"});

    bool noopts = argc == 1;
    auto result = options.parse(argc, argv);

    if (result.count("help") || noopts)
    {
      std::cout << options.help({"", "Group"}) << std::endl;
      return EXIT_SUCCESS;
    }

    // Read all the data from the file
    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetFileName(result["input"].as<std::string>().c_str());
    importer->Update();

    vtkRenderWindow* renderWindow = importer->GetRenderWindow();
    renderWindow->SetSize(1000, 600);

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkNew<vtkF3DInteractor> interactorStyle;
    renderWindowInteractor->SetInteractorStyle(interactorStyle);

    renderWindow->SetWindowName("f3d - The fast 3D viewer");
    renderWindow->Render();
    renderWindowInteractor->Start();
  }
  catch (const cxxopts::OptionException &e)
  {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
