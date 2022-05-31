#ifndef TestSDKHelpers_h
#define TestSDKHelpers_h

#include <filesystem>
#include <iostream>

class TestSDKHelpers
{
public:
  static bool RenderTest(const f3d::image& img, const std::string& baselinePath,
    const std::string& outputPath, const std::string& name, double threshold = 50)
  {
    if (baselinePath.empty() || outputPath.empty() || name.empty())
    {
      std::cerr << "A path or name is empty, aborting" << std::endl;
      return false;
    }

    std::string baseline = baselinePath + name + ".png";
    std::string output = outputPath + name + ".png";
    std::string diff = outputPath + name + ".diff.png";

    if (!std::filesystem::exists(baseline))
    {
      img.save(output);
      std::cerr << "Reference image "
                << baseline + " does not exists, current rendering has been outputted to " << output
                << std::endl;
      return false;
    }

    f3d::image result = img;
    f3d::image diffRes;
    double error;

    if (!result.compare(f3d::image(baseline), threshold, diffRes, error))
    {
      std::cerr << "Current rendering difference with reference image: " << error
                << "  is higher than the threshold of " << threshold << std::endl;
      result.save(output);
      diffRes.save(diff);
      return false;
    }
    return true;
  }

  static bool RenderTest(f3d::window& win, const std::string& baselinePath,
    const std::string& outputPath, const std::string& name, double threshold = 50,
    bool noBackground = false)
  {
    return TestSDKHelpers::RenderTest(
      win.renderToImage(noBackground), baselinePath, outputPath, name, threshold);
  }
};
#endif
