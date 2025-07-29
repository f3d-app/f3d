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
 * Supported formats:
 * - "val, color, val, color, ..."
 * - "val, red, green, blue, val, red, green, blue, ..."
 * Where val is in [0, 1].
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
colormap_t parse(const std::string& str)
{
  // Split by separator
  std::vector<double> colormapVec;
  std::istringstream split(str);
  std::string each;
  while (std::getline(split, each, ','))
  {
    // Parse val into a double
    double val;
    try
    {
      val = options_tools::parse<double>(each);
    }
    catch (const options::parsing_exception&)
    {
      throw options::parsing_exception("Cannot parse value from colormap: " + each +
        ". Check provided colormap string is correct: " + str);
    }

    // Check it is between 0 and 1
    if (val < 0 || val > 1)
    {
      throw options::parsing_exception("Parsed value from colormap: " + each +
        " is not in expected [0, 1] range. Check provided colormap string is correct: " + str);
    }

    // Add value to colormap vector;
    colormapVec.emplace_back(val);

    // recover next string token
    if (!std::getline(split, each, ','))
    {
      throw options::parsing_exception("Incorrect number of tokens in provided colormap: " + str);
    }

    // Try to parse it directly as a color
    f3d::color_t color;
    try
    {
      color = options_tools::parse<f3d::color_t>(each);

      // Add color to colormap vector
      colormapVec.emplace_back(color.r());
      colormapVec.emplace_back(color.g());
      colormapVec.emplace_back(color.b());
      continue;
    }
    catch (const options::parsing_exception&)
    {
      // Quiet catch
    }

    // Not a color, recover next two token, reconstruct r,g,b string and try to parse it as a color
    // again
    std::string green;
    std::string blue;
    if (!std::getline(split, green, ',') || !std::getline(split, blue, ','))
    {
      throw options::parsing_exception("Incorrect number of tokens in provided colormap or a color "
                                       "could not be parsed as expected: " +
        str);
    }

    try
    {
      color = options_tools::parse<f3d::color_t>(each + "," + green + "," + blue);
      colormapVec.emplace_back(color.r());
      colormapVec.emplace_back(color.g());
      colormapVec.emplace_back(color.b());
      continue;
    }
    catch (const options::parsing_exception&)
    {
      throw options::parsing_exception("Cannot parse color from colormap: " + each + "," + green +
        "," + blue + ". Check provided colormap string is correct: " + str);
    }
  }
  return colormap_t(colormapVec);
}

//----------------------------------------------------------------------------
/**
 *  Parse provided string into a transform2d_t
 *  Supported formats:
 *  - "double, double, double, ..." as a sequence of 9 values
 *  - "scale,translation,angle" in any order, default values used if one or two options are omitted
 *    "scale: double", "scale: double,double", "translation: double, double", "angle: double"
 *  Can throw options::parsing_exception in case of failure to parse
 */
template<>
transform2d_t parse(const std::string& str)
{
  const std::string strCompact = std::regex_replace(str, std::regex("\\s"), "");

  if (strCompact[0] >= '0' && strCompact[0] <= '9')
  {
    std::vector<double> input;
    // since the input starts with a numeric char, we try to read as a double vector
    try
    {
      input = parse<std::vector<double>>(strCompact);
    }
    catch (const options::parsing_exception&)
    {
      throw options::parsing_exception("Cannot convert input into a double vector: " + str);
    }

    if (input.size() > 9)
    {
      throw options::parsing_exception(
        "Input vector too large to read as transform2d, size 9 required: " + str);
    }
    else if (input.size() < 9)
    {
      throw options::parsing_exception(
        "Input vector too small to read as transform2d, size 9 required: " + str);
    }

    return transform2d_t(input);
  }

  // try to read with scale/translate/angle notation
  //  read as a scale/translation/angle series
  const std::regex settingCheck(
    "^((scale\\:)|(translation\\:)|(angle\\:)).*", std::regex_constants::icase);
  const std::regex keywordCheck("^((scale)|(translation)|(angle))", std::regex_constants::icase);
  std::smatch matcher;
  std::istringstream split(strCompact);
  std::string each;
  bool setScale = false;
  std::vector<double> scaleVec;
  std::vector<double> translationVec;
  bool angleNext = false;
  std::vector<angle_deg_t> angleVec;
  bool hasScale = false;
  bool hasTranslation = false;
  bool hasAngle = false;

  // split order for parsing: (example "scale:0.1;translation:0.51,2.1;angle:60.0")
  //  - split substrings by ';' ("scale:0.1" "translation:0.51,2.1" "angle:60.0")
  //  - split each substring by ':' (["scale" "0.1"]["translation" "0.51,2.1"]["angle" "60.0"])
  //  - read remaining substrings as keyword or series of double values split by ','

  while (std::getline(split, each, ';'))
  {
    if (std::regex_match(each, matcher, settingCheck))
    {
      std::istringstream subSplit(each);
      std::string subStr;
      while (std::getline(subSplit, subStr, ':'))
      {
        if (std::regex_match(subStr, matcher, keywordCheck))
        {
          // read and apply keyword
          if (subStr == "scale")
          {
            if (hasScale)
            {
              throw options::parsing_exception(
                "Input cannot have multiple scale transforms: " + str);
            }
            setScale = true;
            hasScale = true;
          }
          else if (subStr == "translation")
          {
            if (hasTranslation)
            {
              throw options::parsing_exception(
                "Input cannot have multiple translation transforms: " + str);
            }
            setScale = false;
            hasTranslation = true;
          }
          else if (subStr == "angle")
          {
            if (hasAngle)
            {
              throw options::parsing_exception(
                "Input cannot have multiple angle transforms: " + str);
            }
            angleNext = true;
            hasAngle = true;
          }
        }
        else
        {
          // read as series of double values
          std::istringstream dblSeries(subStr);
          std::string dblStr;
          while (std::getline(dblSeries, dblStr, ','))
          {
            double val;
            try
            {
              val = parse<double>(dblStr);
            }
            catch (const options::parsing_exception&)
            {
              throw options::parsing_exception(
                "Cannot parse input substring " + subStr + " as double series from input: " + str);
            }

            if (angleNext)
            {
              angleVec.emplace_back(val);
            }
            else if (setScale)
            {
              scaleVec.emplace_back(val);
            }
            else
            {
              translationVec.emplace_back(val);
            }
          }
        }
      }
    }
    else
    {
      // option passed without scale/translation/angle keyword
      throw options::parsing_exception(
        "Cannot parse input substring " + each + " as option from input: " + str);
    }
  }

  // input completed, validate vectors and apply
  if (scaleVec.size() > 2)
  {
    throw options::parsing_exception(
      "Too many scale values (" + std::to_string(scaleVec.size()) + ") in input: " + str);
  }
  else if (hasScale && scaleVec.size() == 0)
  {
    throw options::parsing_exception("Scale called without value in input: " + str);
  }

  if (translationVec.size() == 1)
  {
    throw options::parsing_exception("Translation requires two values, single translation value " +
      std::to_string(translationVec[0]) + " in input: " + str);
  }
  else if (translationVec.size() > 2)
  {
    throw options::parsing_exception("Too many translation values (" +
      std::to_string(translationVec.size()) + ") in input: " + str);
  }
  else if (hasTranslation && translationVec.size() == 0)
  {
    throw options::parsing_exception("Translation called without value in input: " + str);
  }

  if (angleVec.size() > 1)
  {
    throw options::parsing_exception(
      "Multiple angle values (" + std::to_string(angleVec.size()) + ") in input: " + str);
  }
  else if (hasAngle && angleVec.size() == 0)
  {
    throw options::parsing_exception("Angle called without value in input: " + str);
  }

  // Clean up vectors to fit with constructor
  if (scaleVec.size() == 0)
  {
    scaleVec = { 1, 1 };
  }
  else if (scaleVec.size() == 1)
  {
    scaleVec.emplace_back(scaleVec.front());
  }
  if (translationVec.size() == 0)
  {
    translationVec = { 0, 0 };
  }
  if (angleVec.size() == 0)
  {
    angleVec.emplace_back(0);
  }

  double_array_t<2> scaleVecOut = double_array_t<2>(scaleVec);
  double_array_t<2> translationVecOut = double_array_t<2>(translationVec);

  return transform2d_t(scaleVecOut, translationVecOut, angleVec[0]);
}

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
std::string format(const color_t& var)
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
std::string format(const direction_t& var)
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
 * Rely on `format(double)` and `format(color_t)`
 */
std::string format(const colormap_t& var)
{
  std::ostringstream stream;
  std::vector<double> vec(var);
  size_t size = vec.size() / 4;
  for (unsigned int i = 0; i < size; i++)
  {
    stream << ((i > 0) ? "," : "") << options_tools::format(vec[i * 4]) << ","
           << options_tools::format(color_t(vec[i * 4 + 1], vec[i * 4 + 2], vec[i * 4 + 3]));
  }
  return stream.str();
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided transform2d_t.
 * Rely on format(std::vector<double>&)
 */
std::string format(const transform2d_t& var)
{
  return options_tools::format(static_cast<std::vector<double>>(var));
}

} // option_tools
} // f3d
#endif // f3d_options_tools_h
