#ifndef TestSDKHelpers_h
#define TestSDKHelpers_h

class TestSDKHelpers
{
public:
  static bool RenderTest(f3d::window& win, const std::string& baselinePath, const std::string& outputPath,
    const std::string& name)
  {
    std::string baseline = baselinePath + name + ".png";
    std::string output = outputPath + name + ".png";
    std::string diff = outputPath + name + ".diff.png";

    f3d::image result = win.renderToImage();
    f3d::image diffRes;

    bool ret = result.compare(f3d::image(baseline), diffRes, 50);
    if (!ret)
    {
      result.save(output);
      diffRes.save(diff);
    }
    return ret;
  }
};
#endif
