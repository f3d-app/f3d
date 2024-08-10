#include "options.h"
#include "options_tools.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include "vtkF3DConfigure.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <variant>

#include <cassert>
namespace f3d
{
//----------------------------------------------------------------------------
class options::internals
{
public:
  template<typename T>
  static void parse(const std::string& str, T& value, option_types)
  {
    internals::parse(str, value);
  }
  static void parse(const std::string& str, double& value, option_types type)
  {
    if (type == option_types::_ratio)
    {
      ratio_t ratio;
      internals::parse(str, ratio);
      value = ratio;
    }
    else
    {
      internals::parse(str, value);
    }
  }

  // TODO expose parse methods in options API ?
  static void parse(const std::string& str, bool& value)
  {
    // TODO implement proper parsing
    bool b1;
    bool b2;
    std::istringstream(str) >> b1;
    std::istringstream(str) >> std::boolalpha >> b2;
    value = b1 || b2;
  }

  static void parse(const std::string& str, int& value)
  {
    // TODO implement proper parsing
    try
    {
      value = std::stoi(str);
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

  static void parse(const std::string& str, double& value)
  {
    // TODO implement proper parsing
    try
    {
      value = std::stod(str);
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

  static void parse(const std::string& str, ratio_t& value)
  {
    double dbl;
    internals::parse(str, dbl);
    value = dbl;
  }

  static void parse(const std::string& str, std::string& value)
  {
    value = str;
  }

  static void parse(const std::string& str, std::vector<double>& value)
  {
    // TODO implement proper parsing
    std::istringstream split(str);
    value.clear();
    for (std::string each; std::getline(split, each, ',');)
    {
      double dbl;
      internals::parse(each, dbl);
      value.push_back(dbl);
    }
  }

  template<typename T>
  static std::string toString(const T& value, option_types)
  {
    return internals::toString(value);
  }
  static std::string toString(const double& value, option_types type)
  {
    if (type == option_types::_ratio)
    {
      ratio_t ratio = value;
      return internals::toString(ratio);
    }
    else
    {
      return internals::toString(value);
    }
  }

  // TODO Improve string generation
  static std::string toString(const bool& var)
  {
    std::stringstream stream;
    stream << std::boolalpha << var;
    return stream.str();
  }

  static std::string toString(const double& var)
  {
    std::ostringstream stream;
    stream << std::noshowpoint << var;
    return stream.str();
  }

  static std::string toString(const f3d::ratio_t& var)
  {
    double val = var;
    return internals::toString(val);
  }

  static std::string toString(const std::string& var)
  {
    return var;
  }

  static std::string toString(const std::vector<double>& var)
  {
    std::ostringstream stream;
    unsigned int i = 0;
    for (auto& elem : var)
    {
      stream << ((i > 0) ? ", " : "") << internals::toString(elem);
      i++;
    }
    return stream.str();
  }

  template<typename T>
  static std::string toString(const T& var)
  {
    return std::to_string(var);
  }

  void setAsString(options& opt, const std::string& name, const std::string& str)
  {
    option_types type = options_tools::getType(name);
    option_variant_t var = options_tools::get(opt, name);
    std::visit([str, type](auto& ref) { internals::parse(str, ref, type); }, var);
    options_tools::set(opt, name, var);
  }

  std::string getAsString(const options& opt, const std::string& name)
  {
    option_types type = options_tools::getType(name);
    option_variant_t var = options_tools::get(opt, name);
    return std::visit([type](const auto& ref) { return internals::toString(ref, type); }, var);
  }
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::internals)
{
  detail::init::initialize();
};

//----------------------------------------------------------------------------
options::~options()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
options::options(const options& opt)
  : Internals(new options::internals)
{
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) noexcept
{
  return *this;
}

//----------------------------------------------------------------------------
options::options(options&& other) noexcept
{
  this->Internals = other.Internals;
  other.Internals = nullptr;
}

//----------------------------------------------------------------------------
options& options::operator=(options&& other) noexcept
{
  if (this != &other)
  {
    delete this->Internals;
    this->Internals = other.Internals;
    other.Internals = nullptr;
  }
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const option_variant_t& value)
{
  options_tools::set(*this, name, value);
  return *this;
}

//----------------------------------------------------------------------------
option_variant_t options::get(const std::string& name) const
{
  return options_tools::get(*this, name);
}

//----------------------------------------------------------------------------
options& options::setAsString(const std::string& name, const std::string& str)
{
  this->Internals->setAsString(*this, name, str);
  return *this;
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name) const
{
  return this->Internals->getAsString(*this, name);
}

//----------------------------------------------------------------------------
options& options::toggle(const std::string& name)
{
  try
  {
    option_variant_t val;
    val = this->get(name);
    this->set(name, !std::get<bool>(val));
    return *this;
  }
  catch (const std::bad_variant_access&)
  {
    throw options::incompatible_exception(
      "Trying to get toggle " + name + " with incompatible type");
  }
}

//----------------------------------------------------------------------------
bool options::isSame(const options& other, const std::string& name) const
{
  return options_tools::get(*this, name) == options_tools::get(other, name);
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, const std::string& name)
{
  options_tools::set(*this, name, options_tools::get(from, name));
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getNames() const
{
  return options_tools::getNames();
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(const std::string& option) const
{
  std::vector<std::string> names = options_tools::getNames();
  if (std::find(names.begin(), names.end(), option) != names.end())
  {
    return { option, 0 };
  }

  std::pair<std::string, int> ret = { "", std::numeric_limits<int>::max() };

  for (const auto& name : names)
  {
    int distance = utils::textDistance(name, option);
    if (distance < ret.second)
    {
      ret = { name, distance };
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
options::parsing_exception::parsing_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
options::incompatible_exception::incompatible_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
options::inexistent_exception::inexistent_exception(const std::string& what)
  : exception(what)
{
}
}
