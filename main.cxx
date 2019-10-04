#include <cstdio>
#include <iostream>

#include "F3DViewer.h"
#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"

#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

int main(int argc, char **argv)
{
  F3DOptions options;
  options.InitializeFromArgs(argc, argv);

  if (!vtksys::SystemTools::FileExists(options.Input))
  {
    cerr << "Specified input file '" << options.Input << "' does not exists." << endl;
    return -1;
  }

  // Read all the data from the file
  vtkNew<vtkF3DGenericImporter> importer;
  importer->SetFileName(options.Input.c_str());
  importer->SetOptions(options);
  importer->Update();

  F3DViewer viewer(&options, importer);
  return viewer.Start();
}
