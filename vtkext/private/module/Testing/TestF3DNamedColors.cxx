#include "F3DStyle.h"
#include <cstdlib>
#include <vtkNew.h>

#include <vtkF3DNamedColors.h>

int TestF3DNamedColors(int argc, char* argv[])
{
  vtkNew<vtkF3DNamedColors> color;
  if (!color->ColorExists("f3d_red") || !color->ColorExists("f3d_green") ||
    !color->ColorExists("f3d_blue") || !color->ColorExists("f3d_yellow") ||
    !color->ColorExists("f3d_white") || !color->ColorExists("f3d_gray") ||
    !color->ColorExists("f3d_black"))
  {
    return EXIT_FAILURE;
  }

  double rgba[4];

  color->GetColor("f3d_red", rgba);
  std::tuple<float, float, float> red = F3DStyle::GetF3DRed();
  if (std::abs(rgba[0] - std::get<0>(red)) > 1e-5 || std::abs(rgba[1] - std::get<1>(red)) > 1e-5 ||
    std::abs(rgba[2] - std::get<2>(red)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_green", rgba);
  std::tuple<float, float, float> green = F3DStyle::GetF3DGreen();
  if (std::abs(rgba[0] - std::get<0>(green)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(green)) > 1e-5 || std::abs(rgba[2] - std::get<2>(green)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_blue", rgba);
  std::tuple<float, float, float> blue = F3DStyle::GetF3DBlue();
  if (std::abs(rgba[0] - std::get<0>(blue)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(blue)) > 1e-5 || std::abs(rgba[2] - std::get<2>(blue)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_yellow", rgba);
  std::tuple<float, float, float> yellow = F3DStyle::GetF3DYellow();
  if (std::abs(rgba[0] - std::get<0>(yellow)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(yellow)) > 1e-5 ||
    std::abs(rgba[2] - std::get<2>(yellow)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_white", rgba);
  std::tuple<float, float, float> white = F3DStyle::GetF3DWhite();
  if (std::abs(rgba[0] - std::get<0>(white)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(white)) > 1e-5 || std::abs(rgba[2] - std::get<2>(white)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_gray", rgba);
  std::tuple<float, float, float> gray = F3DStyle::GetF3DGrey();
  if (std::abs(rgba[0] - std::get<0>(gray)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(gray)) > 1e-5 || std::abs(rgba[2] - std::get<2>(gray)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  color->GetColor("f3d_black", rgba);
  std::tuple<float, float, float> black = F3DStyle::GetF3DBlack();
  if (std::abs(rgba[0] - std::get<0>(black)) > 1e-5 ||
    std::abs(rgba[1] - std::get<1>(black)) > 1e-5 || std::abs(rgba[2] - std::get<2>(black)) > 1e-5)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}