#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DObjectFactory.h"

int TestF3DObjectFactory(int argc, char* argv[])
{
  vtkNew<vtkF3DObjectFactory> factory;
  return EXIT_SUCCESS;
}
