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
  void setAsString(const std::string& name, std::string value)
  {
    option_variant_t var = options_struct_internals::get(this->OptionsStruct, name);
    if (std::holds_alternative<bool>(var))
    {
      // TODO implement proper parsing
      bool b1;
      bool b2;
      std::istringstream(value) >> b1;
      std::istringstream(value) >> std::boolalpha >> b2;
      var = b1 || b2;
    }
    if (std::holds_alternative<int>(var))
    {
      // TODO implement proper parsing
      var = std::stoi(value);
    }
    else if (std::holds_alternative<double>(var))
    {
      // TODO implement proper parsing
      var = std::stod(value);
    }
    else if (std::holds_alternative<ratio_t>(var))
    {
      // TODO implement proper parsing
      var = ratio_t(std::stod(value));
    }
    else if (std::holds_alternative<std::string>(var))
    {
      var = value;
    }
    else if (std::holds_alternative<std::vector<double>>(var))
    {
      // TODO implement proper parsing
      std::istringstream split(value);
      std::vector<double>& vec = std::get<std::vector<double>>(var);
      vec.clear();
      for (std::string each; std::getline(split, each, ','); vec.push_back(std::stod(each)));
    }
    else
    {
      // TODO implement error mgt
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
      if (std::holds_alternative<int>(var))
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
      else
      {
        // TODO implement error mgt
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
options& options::setAsString(const std::string& name, std::string value)
{
  this->Internals->setAsString(name, value);
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
  option_variant_t val;
  val = this->get(name);
  this->set(name, !std::get<bool>(val));
  return *this; 
}

//----------------------------------------------------------------------------
bool options::isSame(const options& other, const std::string& name) const
{
  try
  {
    return options_struct_internals::get(this->Internals->OptionsStruct, name) == options_struct_internals::get(other.Internals->OptionsStruct, name);
  }
  catch (const std::out_of_range&)
  {
    // TODO error mgt
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, const std::string& name)
{
  try
  {
    options_struct_internals::set(this->Internals->OptionsStruct, name, options_struct_internals::get(from.Internals->OptionsStruct, name));
  }
  catch (const std::out_of_range&)
  {
    // TODO error mgt
    std::string error = "Options " + name + " does not exist";
    log::error(error);
    throw options::inexistent_exception(error + "\n");
  }
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
