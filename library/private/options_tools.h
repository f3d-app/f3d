#ifndef f3d_options_tools_h
#define f3d_options_tools_h

#include "options.h"
#include "types.h"
#include "utils.h"

#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <regex>
#include <sstream>

namespace fs = std::filesystem;

namespace f3d
{
namespace options_tools
{

//----------------------------------------------------------------------------
/**
 * A std::string_view trim function
 */
std::string trim(std::string_view strv)
{
  constexpr std::string_view spaces = " \t\r\v\n";
  strv.remove_prefix(std::min(strv.find_first_not_of(spaces), strv.size()));
  strv.remove_suffix(std::min(strv.size() - strv.find_last_not_of(spaces) - 1, strv.size()));
  return std::string(strv);
}

//----------------------------------------------------------------------------
/** Call `std::stod` and throw `std::invalid_argument` if the whole string has not been consumed. */
double stodStrict(const std::string& str)
{
  std::size_t pos = 0;
  const double parsed = std::stod(str, &pos);
  if (pos != str.size())
  {
    throw std::invalid_argument("partial");
  }
  return parsed;
}

//----------------------------------------------------------------------------
/** Call `std::stoi` and throw `std::invalid_argument` if the whole string has not been consumed */
int stoiStrict(const std::string& str)
{
  std::size_t pos = 0;
  const int parsed = std::stoi(str, &pos);
  if (pos != str.size())
  {
    throw std::invalid_argument("partial");
  }
  return parsed;
}

//----------------------------------------------------------------------------
/**
 * Vector specific templated parse method.
 * Splits on `","` and trims chunks before parsing each element
 */
template<typename T>
struct is_vector : std::false_type
{
};
template<typename... Args>
struct is_vector<std::vector<Args...>> : std::true_type
{
};
template<typename T>
T parse(const std::string& str)
{
  static_assert(is_vector<T>::value, "non-vector types parsing must be specialized");

  // TODO implement more parsing possibilities, eg different types of tokens
  T vec;
  std::istringstream split(str);
  for (std::string each; std::getline(split, each, ',');)
  {
    vec.emplace_back(options_tools::parse<typename T::value_type>(options_tools::trim(each)));
  }
  return vec;
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a bool. Supports boolapha values as well as "yes/no" and "on/off"
 */
template<>
bool parse(const std::string& str)
{
  std::string s = str;
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  if (s == "true" || s == "yes" || s == "on" || s == "1")
  {
    return true;
  }
  if (s == "false" || s == "no" || s == "off" || s == "0")
  {
    return false;
  }
  throw options::parsing_exception("Cannot parse " + str + " into a bool");
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into an int
 * Rely on std::stoi for parsing
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
int parse(const std::string& str)
{
  try
  {
    return stoiStrict(str);
  }
  catch (std::invalid_argument const&)
  {
    throw options::parsing_exception("Cannot parse " + str + " into an int");
  }
  catch (std::out_of_range const&)
  {
    throw options::parsing_exception(
      "Cannot parse " + str + " into an int as it would go out of range");
  }
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a double
 * Rely on std::stod for parsing
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
double parse(const std::string& str)
{
  try
  {
    return stodStrict(str);
  }
  catch (std::invalid_argument const&)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a double");
  }
  catch (std::out_of_range const&)
  {
    throw options::parsing_exception(
      "Cannot parse " + str + " into a double as it would go out of range");
  }
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a ratio_t.
 * Supported formats: number (same as `<double>` version), percentage, fraction
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
ratio_t parse(const std::string& str)
{
  try
  {
    if (!str.empty() && str.at(str.size() - 1) == '%')
    {
      return f3d::ratio_t(stodStrict(str.substr(0, str.size() - 1)) / 100);
    }

    const std::size_t sep = str.find_first_of(":/");
    if (sep != std::string::npos)
    {
      return f3d::ratio_t(stodStrict(str.substr(0, sep)) / stodStrict(str.substr(sep + 1)));
    }

    return f3d::ratio_t(stodStrict(str));
  }
  catch (std::invalid_argument const&)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a ratio_t");
  }
  catch (std::out_of_range const&)
  {
    throw options::parsing_exception(
      "Cannot parse " + str + " into a ratio_t as it would go out of range");
  }
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a fs::path using `utils::collapsePath`
 */
template<>
fs::path parse(const std::string& str)
{
  return utils::collapsePath(str);
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a color_t.
 * Supported formats: "R,G,B", "#RRGGBB", "#RGB", rgb(R,G,B)", "hsl(H,S%,L%)", "hsv(H,S%,V%)",
 *                    "hwb(H,W%,B%)", cmyk(C%,M%,Y%,K%), "CSS3 color name"
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
color_t parse(const std::string& str)
{
  const std::string strCompact = std::regex_replace(str, std::regex("\\s"), "");
  double rgb[3]{};

  try
  {
    /* Short hex format search */
    const std::regex shortHexRegex("#([0-9a-f])([0-9a-f])([0-9a-f])", std::regex_constants::icase);
    std::smatch shortHexMatch;
    if (std::regex_match(strCompact, shortHexMatch, shortHexRegex))
    {
      return color_t(
        std::stoul(shortHexMatch[1].str() + shortHexMatch[1].str(), nullptr, 16) / 255.0,
        std::stoul(shortHexMatch[2].str() + shortHexMatch[2].str(), nullptr, 16) / 255.0,
        std::stoul(shortHexMatch[3].str() + shortHexMatch[3].str(), nullptr, 16) / 255.0);
    }

    /* Hex format search */
    const std::regex hexRegex(
      "#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})", std::regex_constants::icase);
    std::smatch hexMatch;
    if (std::regex_match(strCompact, hexMatch, hexRegex))
    {
      return color_t(                                  //
        std::stoul(hexMatch[1], nullptr, 16) / 255.0,  //
        std::stoul(hexMatch[2], nullptr, 16) / 255.0,  //
        std::stoul(hexMatch[3], nullptr, 16) / 255.0); //
    }

    /* RGB format search */
    const std::regex rgbRegex(
      "rgb\\((\\d{1,3}),(\\d{1,3}),(\\d{1,3})\\)", std::regex_constants::icase);
    std::smatch rgbMatch;
    if (std::regex_match(strCompact, rgbMatch, rgbRegex))
    {
      rgb[0] = std::stod(rgbMatch[1]) / 255.0;
      rgb[1] = std::stod(rgbMatch[2]) / 255.0;
      rgb[2] = std::stod(rgbMatch[3]) / 255.0;
      if (rgb[0] > 1.0 || rgb[1] > 1.0 || rgb[2] > 1.0)
      {
        throw options::parsing_exception("Cannot parse " + str + " into a color_t");
      }
      return color_t(rgb[0], rgb[1], rgb[2]);
    }

    /* Hue-based format search: hsl, hsv, hwb */
    const std::regex hueRegex(
      "(hsl|hsv|hwb)\\((\\d{1,3}),(\\d{1,3})%?,(\\d{1,3})%?\\)", std::regex_constants::icase);
    std::smatch hueMatch;
    if (std::regex_match(strCompact, hueMatch, hueRegex))
    {
      const double h = std::stod(hueMatch[2]) / 360.0;
      double s = std::stod(hueMatch[3]) / 100.0;
      double v = std::stod(hueMatch[4]) / 100.0;
      if (h > 1.0 || s > 1.0 || v > 1.0)
      {
        throw options::parsing_exception("Cannot parse " + str + " into a color_t");
      }

      std::string hueFormat = hueMatch[1].str();
      std::transform(hueFormat.begin(), hueFormat.end(), hueFormat.begin(),
        [](unsigned char c) { return std::tolower(c); });
      if (hueFormat == "hsl")
      {
        const double l = v;
        v = l + s * std::min(l, 1.0 - l);
        s = (v == 0.0) ? 0.0 : (2.0 * (1.0 - l / v));
        vtkMath::HSVToRGB(h, s, v, &rgb[0], &rgb[1], &rgb[2]);
        return color_t(rgb[0], rgb[1], rgb[2]);
      }
      if (hueFormat == "hsv")
      {
        vtkMath::HSVToRGB(h, s, v, &rgb[0], &rgb[1], &rgb[2]);
        return color_t(rgb[0], rgb[1], rgb[2]);
      }
      if (hueFormat == "hwb")
      {
        v = 1 - v;
        s = 1 - (s / v);
        vtkMath::HSVToRGB(h, s, v, &rgb[0], &rgb[1], &rgb[2]);
        return color_t(rgb[0], rgb[1], rgb[2]);
      }
    }

    /* CMYK format search */
    const std::regex cmykRegex(
      "cmyk\\((\\d{1,3})%?,(\\d{1,3})%?,(\\d{1,3})%?,(\\d{1,3})%?\\)", std::regex_constants::icase);
    std::smatch cmykMatch;
    if (std::regex_match(strCompact, cmykMatch, cmykRegex))
    {
      const double c = std::stod(cmykMatch[1]) / 100.0;
      const double m = std::stod(cmykMatch[2]) / 100.0;
      const double y = std::stod(cmykMatch[3]) / 100.0;
      const double k = std::stod(cmykMatch[4]) / 100.0;
      if (c > 1.0 || m > 1.0 || y > 1.0 || k > 1.0)
      {
        throw options::parsing_exception("Cannot parse " + str + " into a color_t");
      }
      return color_t(           //
        (1.0 - c) * (1.0 - k),  //
        (1.0 - m) * (1.0 - k),  //
        (1.0 - y) * (1.0 - k)); //
    }

    /* Named colors search */
    vtkNew<vtkNamedColors> color;
    if (color->ColorExists(strCompact))
    {
      double rgba[4];
      color->GetColor(strCompact, rgba);
      return color_t(rgba[0], rgba[1], rgba[2]);
    }

    /* Vector double format */
    return color_t(options_tools::parse<std::vector<double>>(str));
  }
  /* We do not catch std::invalid_argument nor std::out_of_range exception from stod as it is
   * covered by the regex */
  catch (const f3d::type_construction_exception& ex)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a color_t: " + ex.what());
  }
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a direction_t.
 * Supported formats: "X,Y,Z", "[[+|-]X][[+|-]Y][[+|-]Z]" (case insensitive)
 * rely on parse<std::vector<double>>(str)
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
direction_t parse(const std::string& str)
{
  try
  {
    const std::regex re("([+-]?x)?([+-]?y)?([+-]?z)?", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_match(str, match, re))
    {
      direction_t dir;
      int sign = 1;
      for (size_t i = 0; i < 3; ++i)
      {
        const std::string& match_str = match[i + 1].str();
        if (!match_str.empty())
        {
          if (match_str[0] == '-')
          {
            sign = -1;
          }
          else if (match_str[0] == '+')
          {
            sign = +1;
          }
          const int index = std::toupper(match_str[match_str.length() - 1]) - 'X';
          assert(index >= 0 && index < 3);
          dir[index] = sign;
        }
      }
      return dir;
    }
    else
    {
      try
      {
        return direction_t(options_tools::parse<std::vector<double>>(str));
      }
      catch (const options::parsing_exception&)
      {
        throw options::parsing_exception("Cannot parse " + str + " into a direction_t");
      }
    }
  }
  catch (const f3d::type_construction_exception& ex)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a direction_t: " + ex.what());
  }
}

//----------------------------------------------------------------------------
/**
 * Return provided string stripped of leading and trailing spaces.
 */
template<>
std::string parse(const std::string& str)
{
  return options_tools::trim(str);
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a colormap_t.
 * Supported formats: vector of doubles
 * Can throw options::parsing_exception in case of failure to parse
 * TODO add proper parsing
 */
template<>
colormap_t parse(const std::string& str)
{
  try
  {
    return colormap_t(options_tools::parse<std::vector<double>>(str));
  }
  catch (const options::parsing_exception&)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a colormap_t");
  }
}

// TODO Improve string generation
//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided boolean
 * using boolalpha formatting, eg: "true" or "false"
 */
std::string format(bool var)
{
  std::stringstream stream;
  stream << std::boolalpha << var;
  return stream.str();
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided int
 */
std::string format(int var)
{
  return std::to_string(var);
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided double
 * using ostringstream with std::noshowpoint
 */
std::string format(double var)
{
  std::ostringstream stream;
  stream << std::noshowpoint << var;
  return stream.str();
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided ratio_t
 * rely on format(double&)
 */
std::string format(ratio_t var)
{
  // TODO generate a proper ratio string
  return options_tools::format(static_cast<double>(var));
}

//----------------------------------------------------------------------------
/**
 * Generate (returns) a string from provided string
 */
std::string format(const std::string& var)
{
  return var;
}

//----------------------------------------------------------------------------
/**
 * Generate (returns) a string from provided path
 */
std::string format(const fs::path& var)
{
  return var.string();
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided double vector
 * rely on format(double&) and add `, ` between the double values
 */
template<typename T>
std::string format(const std::vector<T>& var)
{
  std::ostringstream stream;
  unsigned int i = 0;
  for (const T& elem : var)
  {
    stream << ((i > 0) ? "," : "") << options_tools::format(elem);
    i++;
  }
  return stream.str();
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided color_t
 * Formats in hex color string "#RRGGBB" if values are multiple of 1/255.
 * Otherwise rely on format(std::vector<double>&)
 */
std::string format(color_t var)
{
  const std::vector<double> colors = { var.r(), var.g(), var.b() };
  if (std::all_of(colors.begin(), colors.end(),
        [](double val)
        {
          return (val >= 0 && val <= 1 &&
            std::fmod(val * 255., 1) < std::numeric_limits<double>::epsilon());
        }))
  {
    std::ostringstream stream;
    stream << "#" << std::hex << std::setfill('0') << std::setw(2)
           << static_cast<int>(colors[0] * 255.) << std::setw(2)
           << static_cast<int>(colors[1] * 255.) << std::setw(2)
           << static_cast<int>(colors[2] * 255.);
    return stream.str();
  }
  else
  {
    return options_tools::format(static_cast<std::vector<double>>(var));
  }
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided direction_t.
 * Format as `+X`/`+X-Y`/... if possible, otherwise rely on `format(std::vector<double>&)`
 */
std::string format(direction_t var)
{
  const auto isZero = [](double a) { return std::abs(a) < 1e-12; };
  const auto absDiff = [](double a, double b) { return std::abs(a) - std::abs(b); };
  const auto formatAsXYZ = [&]()
  {
    std::string str = "";
    double firstNonZero = 0; // not non-zero until first
    char sign = '\0';        // initially not `+`/`-` to force first sign
    for (size_t componentIndex = 0; componentIndex < 3; ++componentIndex)
    {
      const double componentValue = var[componentIndex];
      if (!isZero(componentValue))
      {
        if (isZero(firstNonZero))
        {
          firstNonZero = componentValue;
        }
        else if (!isZero(absDiff(componentValue, firstNonZero)))
        {
          throw std::invalid_argument("not all same");
        }
        const char newSign = componentValue < 0 ? '-' : '+';
        if (newSign != sign)
        {
          str += newSign;
          sign = newSign;
        }
        str += static_cast<char>('X' + componentIndex);
      }
    }
    if (str.empty())
    {
      throw std::invalid_argument("all zeroes");
    }
    return str;
  };

  try
  {
    return formatAsXYZ();
  }
  catch (const std::invalid_argument&)
  {
    return options_tools::format(static_cast<std::vector<double>>(var));
  }
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided colormap_t.
 * Rely on `format(std::vector<double>&)`
 * TODO add proper formatting
 */
std::string format(const colormap_t& var)
{
  return options_tools::format(static_cast<std::vector<double>>(var));
}

} // option_tools
} // f3d
#endif // f3d_options_tools_h
