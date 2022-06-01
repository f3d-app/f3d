#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOpenGLGridMapper.h"

#include <iostream>

int TestF3DOpenGLGridMapper(int argc, char* argv[])
{
  vtkNew<vtkF3DOpenGLGridMapper> mapper;

  mapper->SetFadeDistance(5.0);
  mapper->Print(std::cout);

  double* bounds = mapper->GetBounds();
  if (bounds[0] != -5.0 || bounds[1] != 5.0)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
