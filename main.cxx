#include <cstdio>
#include <iostream>

#include "F3DViewer.h"
#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"

#include <vtkNew.h>

int main(int argc, char **argv)
{
  F3DOptions options;
  options.InitializeFromArgs(argc, argv);

  // Read all the data from the file
  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetFileName(options.Input.c_str());
  importer->Update();

  F3DViewer viewer(&options, importer);
  viewer.Start();

  return EXIT_SUCCESS;
}
