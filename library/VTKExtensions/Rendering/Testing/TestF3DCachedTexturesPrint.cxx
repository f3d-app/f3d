#include <vtkNew.h>
#include <vtkRenderer.h>

#include "vtkF3DCachedLUTTexture.h"
#include "vtkF3DCachedSpecularTexture.h"

int TestF3DCachedTexturesPrint(int argc, char* argv[])
{
  vtkNew<vtkF3DCachedLUTTexture> lut;
  vtkNew<vtkF3DCachedSpecularTexture> specular;

  lut->Print(cout);
  specular->Print(cout);

  return EXIT_SUCCESS;
}
