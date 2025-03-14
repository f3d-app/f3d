#include "F3DUtils.h"

#include <vtkObject.h>
#include <vtkSetGet.h>

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
