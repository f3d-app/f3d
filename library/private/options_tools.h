#ifndef f3d_options_tools_h
#define f3d_options_tools_h

#include "options.h"
#include "types.h"

#include <algorithm>
#include <cassert>
#include <regex>
#include <sstream>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkSmartPointer.h>

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
 * Parse provided string into a color_t.
 * Supported formats: "R,G,B"
 * rely on parse<std::vector<double>>(str)
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
color_t parse(const std::string& str)
{
  const std::string s = std::regex_replace(str, std::regex("\\s"), "");

  /* Hex format search */
  const std::regex hexRegex(
    "^#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$", std::regex_constants::icase);
  std::smatch hexMatch;
  if (std::regex_search(s, hexMatch, hexRegex))
  {
    return color_t(                                  //
      std::stoul(hexMatch[1], nullptr, 16) / 255.0,  //
      std::stoul(hexMatch[2], nullptr, 16) / 255.0,  //
      std::stoul(hexMatch[3], nullptr, 16) / 255.0); //
  }

  /* RGB format search */
  const std::regex rgbRegex(
    "^rgb\\((\\d{1,3}),(\\d{1,3}),(\\d{1,3})\\)$", std::regex_constants::icase);
  std::smatch rgbMatch;
  if (std::regex_search(s, rgbMatch, rgbRegex))
  {
    return color_t(                    //
      std::stod(rgbMatch[1]) / 255.0,  //
      std::stod(rgbMatch[2]) / 255.0,  //
      std::stod(rgbMatch[3]) / 255.0); //
  }

  /* Hue-based format search: hsl, hsv, hwb */
  const std::regex hueRegex(
    "^([a-z]{3})\\((\\d{1,3}),(\\d{1,3})%{0,1},(\\d{1,3})%{0,1}\\)$", std::regex_constants::icase);
  std::smatch hueMatch;
  if (std::regex_search(s, hueMatch, hueRegex))
  {
    double rgb[3]{};
    std::string hueFormat = hueMatch[1].str();
    std::transform(hueFormat.begin(), hueFormat.end(), hueFormat.begin(),
      [](unsigned char c) { return std::tolower(c); });
    if (hueFormat == "hsl")
    {
      double sl = std::stod(hueMatch[3]) / 100.0;
      double l = std::stod(hueMatch[4]) / 100.0;
      double v = l + sl * std::min(l, 1.0 - l);
      double s = (v == 0.0) ? 0.0 : (2.0 * (1.0 - l / v));
      vtkMath::HSVToRGB(                //
        std::stod(hueMatch[2]) / 360.0, //
        s,                              //
        v,                              //
        &rgb[0],                        //
        &rgb[1],                        //
        &rgb[2]);                       //
      return color_t(rgb[0], rgb[1], rgb[2]);
    }
    if (hueFormat == "hsv")
    {
      vtkMath::HSVToRGB(                //
        std::stod(hueMatch[2]) / 360.0, //
        std::stod(hueMatch[3]) / 100.0, //
        std::stod(hueMatch[4]) / 100.0, //
        &rgb[0],                        //
        &rgb[1],                        //
        &rgb[2]);                       //
      return color_t(rgb[0], rgb[1], rgb[2]);
    }
    if (hueFormat == "hwb")
    {
      double w = std::stod(hueMatch[3]) / 100.0;
      double b = std::stod(hueMatch[4]) / 100.0;
      double v = 1 - b;
      double s = 1 - (w / v);
      vtkMath::HSVToRGB(                //
        std::stod(hueMatch[2]) / 360.0, //
        s,                              //
        v,                              //
        &rgb[0],                        //
        &rgb[1],                        //
        &rgb[2]);                       //
      return color_t(rgb[0], rgb[1], rgb[2]);
    }
  }

  /* Named colors search */
  vtkSmartPointer<vtkNamedColors> color = vtkSmartPointer<vtkNamedColors>::New();
  if (color->ColorExists(s))
  {
    double rgba[4];
    color->GetColor(s, rgba);
    return color_t(rgba[0], rgba[1], rgba[2]);
  }

  try
  {
    return color_t(options_tools::parse<std::vector<double>>(str));
  }
  catch (const f3d::type_construction_exception& ex)
  {
    throw options::parsing_exception("Cannot parse " + str + " into a color_t: " + ex.what());
  }
}

//----------------------------------------------------------------------------
/**
 * Parse provided string into a direction_t.
 * Supported formats: "X,Y,Z", "[+|-][X|Y|Z]"
 * rely on parse<std::vector<double>>(str)
 * Can throw options::parsing_exception in case of failure to parse
 */
template<>
direction_t parse(const std::string& str)
{
  try
  {
    const std::regex re("([-+]?)([XYZ])", std::regex_constants::icase);
    std::smatch match;
    if (std::regex_match(str, match, re))
    {
      const double sign = match[1].str() == "-" ? -1.0 : +1.0;
      const int index = std::toupper(match[2].str()[0]) - 'X';
      assert(index >= 0 && index < 3);

      direction_t dir;
      dir[index] = sign;
      return dir;
    }
    else
    {
      return direction_t(options_tools::parse<std::vector<double>>(str));
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
 * rely on format(std::vector<double>&)
 */
std::string format(color_t var)
{
  // TODO generate a proper color string
  return options_tools::format(static_cast<std::vector<double>>(var));
}

//----------------------------------------------------------------------------
/**
 * Format provided var into a string from provided direction_t
 * rely on format(std::vector<double>&)
 */
std::string format(direction_t var)
{
  // TODO generate a proper direction string
  return options_tools::format(static_cast<std::vector<double>>(var));
}

} // option_tools
} // f3d
#endif // f3d_options_tools_h
