#include "F3DColorMapTools.h"

#include "F3DSystemTools.h"

#include "image.h"
#include "log.h"
#include "utils.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace F3DColorMapTools
{
fs::path Find(const std::string& str)
{
  try
  {
    fs::path fullPath(f3d::utils::collapsePath(str));
    if (fs::exists(fullPath))
    {
      if (fs::is_regular_file(fullPath))
      {
        // already full path
        return fullPath;
      }
    }

    std::vector<fs::path> dirsToCheck{ F3DSystemTools::GetUserConfigFileDirectory() / "colormaps",
#ifdef __APPLE__
      "/usr/local/etc/f3d/colormaps",
#endif
#ifdef __linux__
      "/etc/f3d/colormaps", "/usr/share/f3d/colormaps",
#endif
      F3DSystemTools::GetBinaryResourceDirectory() / "colormaps" };

    for (const fs::path& dir : dirsToCheck)
    {
      // If the string is a stem, try adding supported extensions
      if (fs::path(str).stem() == str)
      {
        for (const std::string& ext : f3d::image::getSupportedFormats())
        {
          fs::path cmPath = dir / (str + ext);
          if (fs::exists(cmPath))
          {
            return cmPath;
          }
        }
      }
      else
      {
        // If not, use directly
        fs::path cmPath = dir / str;
        if (fs::exists(cmPath))
        {
          return cmPath;
        }
      }
    }
  }
  catch (const fs::filesystem_error& ex)
  {
    f3d::log::error("Unable to look for color map ", str, ": ", ex.what());
  }

  return {};
}

std::vector<double> Read(const fs::path& path)
{
  try
  {
    f3d::image img(path);

    if (img.getChannelCount() < 3)
    {
      f3d::log::error("The specified color map must have at least 3 channels");
      return {};
    }

    if (img.getHeight() != 1)
    {
      f3d::log::warn("The specified color map height is not equal to 1, only the first row is "
                     "taken into account");
    }

    int w = img.getWidth();

    std::vector<double> cm(4 * w);

    for (int i = 0; i < w; i++)
    {
      std::vector<double> pixel = img.getNormalizedPixel({ i, 0 });
      cm[4 * i + 0] = static_cast<double>(i) / (w - 1);
      cm[4 * i + 1] = pixel[0];
      cm[4 * i + 2] = pixel[1];
      cm[4 * i + 3] = pixel[2];
    }

    return cm;
  }
  catch (const f3d::image::read_exception&)
  {
    f3d::log::error("Cannot read colormap at ", path);
    return {};
  }
}
}
