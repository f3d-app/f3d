#ifndef TestSDKHelpers_h
#define TestSDKHelpers_h

#include <fstream>
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

    {
      std::ifstream file(baseline.c_str());
      if (!file.good())
      {
        img.save(output);
        std::cerr << "Reference image "
                  << baseline + " does not exist, current rendering has been outputted to "
                  << output << std::endl;
        return false;
      }
    }

    f3d::image diffRes;
    f3d::image reference(baseline);
    double error;

    if (!img.compare(reference, threshold, diffRes, error))
    {
      std::cerr << "Current rendering difference with reference image: " << error
                << " is higher than the threshold of " << threshold << std::endl;
      std::cerr << "Result resolution: " << img.getWidth() << "x" << img.getHeight() << std::endl;
      std::cerr << "Reference resolution: " << reference.getWidth() << "x" << reference.getHeight()
                << std::endl;
      img.save(output);
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
