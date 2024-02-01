#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOpenGLGridMapper.h"

#include <iostream>
#include <sstream>

std::string boundsToStr(const double* bounds)
{
  std::stringstream ss;
  ss << "(" << bounds[0] << "," << bounds[1] << "," << bounds[2] << "," << bounds[3] << ","
     << bounds[4] << "," << bounds[5] << ")";
  return ss.str();
}

bool checkBounds(const std::string& name, vtkF3DOpenGLGridMapper* mapper, //
  double x0, double x1, double y0, double y1, double z0, double z1)
{
  const double* actual = mapper->GetBounds();
  const double expected[6] = { x0, x1, y0, y1, z0, z1 };
  for (size_t i = 0; i < 6; ++i)
  {
    if (actual[i] != expected[i])
    {
      std::cerr << "wrong bounds for " << name << " failed:" << std::endl;
      std::cerr << "  got " << boundsToStr(actual) << std::endl;
      std::cerr << "  expected " << boundsToStr(expected) << std::endl;
      return false;
    }
  }
  return true;
}

int TestF3DOpenGLGridMapper(int argc, char* argv[])
{
  {
    vtkNew<vtkF3DOpenGLGridMapper> mapper;

    mapper->SetFadeDistance(5.0);
    mapper->Print(std::cout);

    if (!checkBounds("default", mapper, -5, +5, 0, 0, -5, +5))
      return EXIT_FAILURE;
  }

  {
    const double r = 8.0;

    vtkNew<vtkF3DOpenGLGridMapper> mapper;
    mapper->SetFadeDistance(r);
    mapper->SetOriginOffset(1, -2, 3);
    /* `OriginOffset` offset is only for drawing the axes within the actor,
     * it should not affect the actual bounding box */

    mapper->SetUpIndex(0);
    if (!checkBounds("YZ with offset", mapper, 0, 0, -r, +r, -r, +r))
      return EXIT_FAILURE;

    mapper->SetUpIndex(1);
    if (!checkBounds("XZ with offset", mapper, -r, +r, 0, 0, -r, +r))
      return EXIT_FAILURE;

    mapper->SetUpIndex(2);
    if (!checkBounds("XY with offset", mapper, -r, +r, -r, +r, 0, 0))
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
