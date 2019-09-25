#include <cstdio>
#include <iostream>

#include "cxxopts.hpp"

#include "vtkF3DInteractor.h"
#include "vtkF3DGenericImporter.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

int main(int argc, char **argv)
{
  std::string title = "f3d - Fast and minimalist 3D viewer";
  try
  {
    cxxopts::Options options(argv[0], title);
    options
      .positional_help("input_file")
      .show_positional_help();

    bool normals = false;
    bool axis = false;
    std::vector<int> size;

    options
      .add_options()
      ("n,normals", "Show mesh normals", cxxopts::value<bool>(normals))
      ("i,input", "Input file", cxxopts::value<std::string>(), "file")
      ("h,help", "Print help")
      ("x,axis", "Show axis", cxxopts::value<bool>(axis))
      ("s,size", "Window size", cxxopts::value<std::vector<int>>(size)->default_value("1000,600"));

    options.parse_positional({"input", "positional"});

    if (argc == 1)
    {
      std::cout << options.help() << std::endl;
      return EXIT_FAILURE;
    }

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0)
    {
      std::cout << options.help() << std::endl;
      return EXIT_SUCCESS;
    }

    // Read all the data from the file
    vtkNew<vtkF3DGenericImporter> importer;
    importer->SetFileName(result["input"].as<std::string>().c_str());
    importer->Update();

    vtkRenderWindow* renderWindow = importer->GetRenderWindow();
    renderWindow->SetSize(size[0], size[1]);

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkNew<vtkF3DInteractor> interactorStyle;
    renderWindowInteractor->SetInteractorStyle(interactorStyle);

    renderWindow->SetWindowName(title.c_str());
    renderWindow->Render();

    vtkNew<vtkOrientationMarkerWidget> widget;
    if (axis)
    {
      vtkNew<vtkAxesActor> axes;
      widget->SetOrientationMarker(axes);
      widget->SetInteractor(renderWindowInteractor);
      widget->SetViewport(0.85, 0.0, 1.0, 0.15);
      widget->SetEnabled(1);
      widget->InteractiveOff();
    }

    renderWindowInteractor->Start();
  }
  catch (const cxxopts::OptionException &e)
  {
    std::cout << "error parsing options: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
