#include "F3DColorMapTools.h"

#include "F3DSystemTools.h"

#include "image.h"
#include "log.h"
#include "utils.h"

#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

namespace F3DColorMapTools
{
enum class MapType : std::uint8_t
{
  Color,
  Opacity
};

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
#if defined(__linux__) || defined(__FreeBSD__)
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

std::vector<double> Read1DMap(const fs::path& path, MapType type)
{
  try
  {
    f3d::image img(path);

    const int channels = img.getChannelCount();
    const int expectedChannels = (type == MapType::Color) ? 3 : 1;

    if (channels < expectedChannels)
    {
      f3d::log::error("The specified ", (type == MapType::Color ? "color" : "opacity"),
        " map must have at least ", expectedChannels,
        " channel" + std::string(expectedChannels > 1 ? "s" : ""));
      return {};
    }

    if (img.getHeight() != 1)
    {
      f3d::log::warn("The specified ", (type == MapType::Color ? "color" : "opacity"),
        " map height is not equal to 1, only the first row is taken into account");
    }

    const int w = img.getWidth();
    const int stride = (type == MapType::Color) ? 4 : 2;

    std::vector<double> out(stride * w);

    for (int i = 0; i < w; i++)
    {
      const auto pixel = img.getNormalizedPixel({ i, 0 });
      const double x = static_cast<double>(i) / (w - 1);
      out[stride * i + 0] = x;
      for (int c = 1; c <= expectedChannels; c++)
      {
        out[stride * i + c] = pixel[c - 1];
      }
    }

    return out;
  }
  catch (const f3d::image::read_exception&)
  {
    f3d::log::error(
      "Cannot read ", (type == MapType::Color ? "colormap" : "opacity map"), " at ", path);
    return {};
  }
}

f3d::colormap_t Read(const fs::path& path)
{
  return f3d::colormap_t(F3DColorMapTools::Read1DMap(path, MapType::Color));
}

std::vector<double> ReadOpacity(const fs::path& path)
{
  return F3DColorMapTools::Read1DMap(path, MapType::Opacity);
}
}
