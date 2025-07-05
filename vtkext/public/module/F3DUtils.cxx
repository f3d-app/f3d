#include "F3DUtils.h"

#include <vtkObject.h>
#include <vtkSetGet.h>

#include <charconv>
#include <stdexcept>

//----------------------------------------------------------------------------
double F3DUtils::ParseToDouble(const std::string& str, double def, const std::string& nameError)
{
  double value = def;
  if (!str.empty())
  {
    try
    {
      value = std::stod(str);
    }
    catch (const std::invalid_argument&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
    }
    catch (const std::out_of_range&)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
    }
  }
  return value;
}

//----------------------------------------------------------------------------
int F3DUtils::ParseToInt(const std::string& str, int def, const std::string& nameError)
{
  int value = def;
  if (!str.empty())
  {
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);

    if (result.ec == std::errc::result_out_of_range)
    {
      vtkWarningWithObjectMacro(
        nullptr, "Provided " << nameError << " out of range: " << str << ". Ignoring.");
      return def;
    }
    else if (result.ec != std::errc() || *(result.ptr) != '\0')
    {
      vtkWarningWithObjectMacro(
        nullptr, "Could not parse " << nameError << ": " << str << ". Ignoring.");
      return def;
    }
  }
  return value;
}
