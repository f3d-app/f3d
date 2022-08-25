#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DRenderer.h"

int TestF3DRenderer(int argc, char* argv[])
{
  vtkNew<vtkF3DRenderer> ren;

  // Check some invalid code path
  ren->ShowGrid(true);
  return EXIT_SUCCESS;
}
