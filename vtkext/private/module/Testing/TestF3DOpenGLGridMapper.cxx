#include <vtkNew.h>
#include <vtkTestUtilities.h>

#include "vtkF3DOpenGLGridMapper.h"

#include <array>
#include <iostream>

template<class T, std::size_t N>
ostream& operator<<(std::ostream& o, const std::array<T, N>& arr)
{
  size_t i = 0;
  o << "(";
  for (const auto v : arr)
  {
    if (i++)
    {
      o << ", ";
    }
    o << v;
  }
  o << ")";
  return o;
}

bool CheckBounds(const std::string& name, vtkF3DOpenGLGridMapper* mapper, //
  double x0, double x1, double y0, double y1, double z0, double z1)
{
  std::array<double, 6> actual;
  mapper->GetBounds(actual.data());
  const std::array<double, 6> expected = { x0, x1, y0, y1, z0, z1 };
  if (actual != expected)
  {
    std::cerr << "wrong bounds for " << name << " failed:\n";
    std::cerr << "  got " << actual << "\n";
    std::cerr << "  expected " << expected << "\n";
    return false;
  }
  return true;
}

int TestF3DOpenGLGridMapper(int argc, char* argv[])
{
  constexpr double safeMargin = 1e-4;

  {
    vtkNew<vtkF3DOpenGLGridMapper> mapper;

    mapper->SetFadeDistance(5.0);
    mapper->Print(std::cout);

    if (!CheckBounds("default", mapper, -5, +5, -safeMargin, +safeMargin, -5, +5))
    {
      return EXIT_FAILURE;
    }
  }

  {
    const double r = 8.0;

    vtkNew<vtkF3DOpenGLGridMapper> mapper;
    mapper->SetFadeDistance(r);
    mapper->SetOriginOffset(1, -2, 3);
    /* `OriginOffset` offset is only for drawing the axes within the actor,
     * it should not affect the actual bounding box */

    if (!CheckBounds("XZ with offset", mapper, -r, +r, -safeMargin, +safeMargin, -r, +r))
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
