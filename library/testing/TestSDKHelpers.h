#ifndef TestSDKHelpers_h
#define TestSDKHelpers_h

#include <image.h>
#include <window.h>

#include <cmath>
#include <fstream>
#include <iostream>

namespace TestSDKHelpers
{

inline bool RenderTest(const f3d::image& img, const std::string& baselinePath,
  const std::string& outputPath, const std::string& name, double threshold = 0.05)
{
  if (baselinePath.empty() || outputPath.empty() || name.empty())
  {
    std::cerr << "A path or name is empty, aborting\n";
    return false;
  }

  std::string baseline = baselinePath + name + ".png";
  std::string output = outputPath + name + ".png";

  {
    std::ifstream file(baseline.c_str());
    if (!file.good())
    {
      img.save(output);
      std::cerr << "Reference image "
                << baseline + " does not exist, current rendering has been outputted to " << output
                << "\n";
      return false;
    }
  }

  f3d::image reference(baseline);

  double error = img.compare(reference);

  if (error > threshold)
  {
    std::cerr << "Current rendering difference with reference image " << baseline << " : " << error
              << " is higher than the threshold of " << threshold << "\n";
    std::cerr << "Result resolution: " << img.getWidth() << "x" << img.getHeight() << "\n";
    std::cerr << "Reference resolution: " << reference.getWidth() << "x" << reference.getHeight()
              << "\n";
    std::cerr << "Result image saved to " << output << "\n";
    img.save(output);
    return false;
  }

  std::cout << "Successful render test against " << baseline + " with an error of " << error
            << "\n";
  return true;
}

inline bool RenderTest(f3d::window& win, const std::string& baselinePath,
  const std::string& outputPath, const std::string& name, double threshold = 0.05,
  bool noBackground = false)
{
  return TestSDKHelpers::RenderTest(
    win.renderToImage(noBackground), baselinePath, outputPath, name, threshold);
}

constexpr double Degrees2Radians(double degrees)
{
  return (3.14159265358979323846 * degrees / 180.0);
}

constexpr bool CompareDouble(double a, double b)
{
  return std::fabs(a - b) < 128 * std::numeric_limits<double>::epsilon();
}

constexpr bool CompareVec(const f3d::vector3_t& vec1, const f3d::vector3_t& vec2)
{
  return TestSDKHelpers::CompareDouble(vec1[0], vec2[0]) &&
    TestSDKHelpers::CompareDouble(vec1[1], vec2[1]) &&
    TestSDKHelpers::CompareDouble(vec1[2], vec2[2]);
}

constexpr bool ComparePoint(const f3d::point3_t& vec1, const f3d::point3_t& vec2)
{
  return TestSDKHelpers::CompareDouble(vec1[0], vec2[0]) &&
    TestSDKHelpers::CompareDouble(vec1[1], vec2[1]) &&
    TestSDKHelpers::CompareDouble(vec1[2], vec2[2]);
}
}
#endif
