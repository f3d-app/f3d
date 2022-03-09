#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DRenderPass.h"

int TestF3DRenderPass(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderPass> pass;
  return EXIT_SUCCESS;
}
