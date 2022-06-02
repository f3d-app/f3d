#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DRenderPass.h"

#include <iostream>

int TestF3DRenderPass(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderPass> pass;
  pass->Print(std::cout);
  return EXIT_SUCCESS;
}
