#include "F3DStyle.h"
#include "vtkF3DNamedColors.h"

#include <vtkNew.h>

#include <cmath>

bool TestColor(const std::string& name, const std::tuple<float, float, float>& rgb)
{
  vtkNew<vtkF3DNamedColors> color;
  if (!color->ColorExists(name))
  {
    return false;
  }

  double rgba[4];
  color->GetColor(name, rgba);
  return (std::abs(rgba[0] - std::get<0>(rgb)) < 1e-5 ||
    std::abs(rgba[1] - std::get<1>(rgb)) < 1e-5 || std::abs(rgba[2] - std::get<2>(rgb)) < 1e-5);
}

int TestF3DNamedColors(int argc, char* argv[])
{
  if (!TestColor("f3d_red", F3DStyle::GetF3DRed()) ||
    !TestColor("f3d_green", F3DStyle::GetF3DGreen()) ||
    !TestColor("f3d_blue", F3DStyle::GetF3DBlue()) ||
    !TestColor("f3d_yellow", F3DStyle::GetF3DYellow()) ||
    !TestColor("f3d_white", F3DStyle::GetF3DWhite()) ||
    !TestColor("f3d_grey", F3DStyle::GetF3DGrey()) ||
    !TestColor("f3d_black", F3DStyle::GetF3DBlack()))
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
