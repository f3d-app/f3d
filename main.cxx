#include <cstdio>
#include <iostream>

#include "cxxopts.hpp"

#include "vtkF3DInteractor.h"
#include "vtkF3DMetaReader.h"

#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderer.h>
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

    if (result.count("input"))
    {
      std::cout << "Input = " << result["input"].as<std::string>()
                << std::endl;
    }

    // Read all the data from the file
    vtkNew<vtkF3DMetaReader> reader;
    reader->SetFileName(result["input"].as<std::string>());
    reader->Update();

    // Visualize
    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkNew<vtkF3DInteractor> interactorStyle;
    renderWindowInteractor->SetInteractorStyle(interactorStyle);

    renderer->AddActor(actor);
    renderer->SetBackground(.4, .4, .4);

    renderWindow->SetWindowName("f3d");
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
