#include <cstdio>
#include <iostream>

#include "F3DLoader.h"
#include "F3DOptions.h"
#include "F3DViewer.h"


int main(int argc, char** argv)
{
  F3DOptions options;
  options.InitializeFromArgs(argc, argv);

  F3DLoader loader(options);

  F3DViewer viewer(&options, loader.GetImporter());
  return viewer.Start();
}
