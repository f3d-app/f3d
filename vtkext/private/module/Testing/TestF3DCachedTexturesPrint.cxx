#include <vtkNew.h>
#include <vtkRenderer.h>

#include "vtkF3DCachedLUTTexture.h"
#include "vtkF3DCachedSpecularTexture.h"

#include <iostream>

int TestF3DCachedTexturesPrint(int argc, char* argv[])
{
  vtkNew<vtkF3DCachedLUTTexture> lut;
  vtkNew<vtkF3DCachedSpecularTexture> specular;

  lut->Print(std::cout);
  specular->Print(std::cout);

  return EXIT_SUCCESS;
}
