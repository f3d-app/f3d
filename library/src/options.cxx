#include "options.h"
#include "options_struct.h"
#include "options_struct_internals.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include "vtkF3DConfigure.h"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <iostream>

namespace f3d
{
//----------------------------------------------------------------------------
class options::internals
{
public:
  void parse(std::string str, bool& value)
  {
    // TODO implement proper parsing
    bool b1;
    bool b2;
    std::istringstream(str) >> b1;
    std::istringstream(str) >> std::boolalpha >> b2;
    value = b1 || b2;
  }

  void parse(std::string str, int& value)
  {
    // TODO implement proper parsing
    try
    {
      value = std::stoi(str);
    }
    catch (std::invalid_argument const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into an int");
    }
    catch (std::out_of_range const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into an int as it would go out of range");
    }
  }

  void parse(std::string str, double& value)
  {
    // TODO implement proper parsing
    try
    {
      value = std::stod(str);
    }
    catch (std::invalid_argument const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a double");
    }
    catch (std::out_of_range const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a double as it would go out of range");
    }
  }

  void parse(std::string str, ratio_t& value)
  {
    // TODO implement proper parsing
    try
    {
      value = std::stod(str);
    }
    catch (std::invalid_argument const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a ratio");
    }
    catch (std::out_of_range const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a ratio as it would go out of double range");
    }
  }

  void parse(std::string str, std::string& value)
  {
    value = str;
  }

  void parse(std::string str, std::vector<double>& value)
  {
    // TODO implement proper parsing
    try
    {
      std::istringstream split(str);
      value.clear();
      for (std::string each; std::getline(split, each, ','); value.push_back(std::stod(each)));
    }
    catch (std::invalid_argument const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a double vector");
    }
    catch (std::out_of_range const& ex)
    {
      throw options::parsing_exception("Cannot parse " + str + " into a double vector as it would go out of double range");
    }
  }

  void setAsString(const std::string& name, std::string str)
  {
    option_variant_t var = options_struct_internals::get(this->OptionsStruct, name);

    // TODO Use std::visit ?
    if (std::holds_alternative<bool>(var))
    {
      bool value;
      this->parse(str, value);
      var = value;
    }
    else if (std::holds_alternative<int>(var))
    {
      int value;
      this->parse(str, value);
      var = value;
    }
    else if (std::holds_alternative<double>(var))
    {
      double value;
      this->parse(str, value);
      var = value;
    }
    else if (std::holds_alternative<ratio_t>(var))
    {
      ratio_t value(0);
      this->parse(str, value);
      var = value;
    }
    else if (std::holds_alternative<std::string>(var))
    {
      std::string value;
      this->parse(str, value);
      var = value;
    }
    else if (std::holds_alternative<std::vector<double>>(var))
    {
      std::vector<double> value;
      this->parse(str, value);
      var = value;
    }
    options_struct_internals::set(this->OptionsStruct, name, var);
  }

  std::string getAsString(const std::string& name)
  {
    option_variant_t var = options_struct_internals::get(this->OptionsStruct, name);
    std::string str;
    try
    {
      if (std::holds_alternative<bool>(var))
      {
        str = std::to_string(std::get<bool>(var));
      }
      else if (std::holds_alternative<int>(var))
      {
        str = std::to_string(std::get<int>(var));
      }
      else if (std::holds_alternative<double>(var))
      {
        str = std::to_string(std::get<double>(var));
      }
      else if (std::holds_alternative<ratio_t>(var))
      {
        str = std::to_string(std::get<ratio_t>(var));
      }
      else if (std::holds_alternative<std::string>(var))
      {
        str = std::get<std::string>(var);
      }
      else if (std::holds_alternative<std::vector<double>>(var))
      {
        std::vector<double> vec = std::get<std::vector<double>>(var);
        std::ostringstream stream;
        unsigned int i = 0;
        for (auto& elem : vec)
        {
          stream << ((i > 0) ? "," : "") << std::to_string(elem);
          i++;
        }
        stream << '\n';
        str = stream.str();
      }
    }
    catch (const std::bad_variant_access&)
    {
      throw options::incompatible_exception(
        "Trying to get option reference " + name + " with incompatible type");
    }
    return str;
  }
  options_struct OptionsStruct;
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
  this->Internals->OptionsStruct = opt.Internals->OptionsStruct;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) noexcept
{
  this->Internals->OptionsStruct = opt.Internals->OptionsStruct;
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
options& options::set(const std::string& name, option_variant_t value)
{
  options_struct_internals::set(this->Internals->OptionsStruct, name, value);
  return *this;
}

//----------------------------------------------------------------------------
option_variant_t options::get(const std::string& name)
{
  return options_struct_internals::get(this->Internals->OptionsStruct, name);
}

//----------------------------------------------------------------------------
options& options::setAsString(const std::string& name, std::string str)
{
  this->Internals->setAsString(name, str);
  return *this;
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name)
{
  return this->Internals->getAsString(name);
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
  return options_struct_internals::get(this->Internals->OptionsStruct, name) == options_struct_internals::get(other.Internals->OptionsStruct, name);
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, const std::string& name)
{
  options_struct_internals::set(this->Internals->OptionsStruct, name, options_struct_internals::get(from.Internals->OptionsStruct, name));
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getNames()
{
  return options_struct_internals::getNames();
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(const std::string& option) const
{
  std::vector<std::string> names = options_struct_internals::getNames();
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

//----------------------------------------------------------------------------
options_struct& options::getStruct()
{
  return this->Internals->OptionsStruct;
}

//----------------------------------------------------------------------------
const options_struct& options::getConstStruct() const
{
  return this->Internals->OptionsStruct;
}
}
