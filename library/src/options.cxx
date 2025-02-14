#include "options.h"
#include "options_tools.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include <algorithm>
#include <limits>
#include <string>

namespace f3d
{
//----------------------------------------------------------------------------
options::options()
{
  detail::init::initialize();
};

//----------------------------------------------------------------------------
options& options::set(std::string_view name, const option_variant_t& value)
{
  options_tools::set(*this, name, value);
  return *this;
}

//----------------------------------------------------------------------------
option_variant_t options::get(std::string_view name) const
{
  return options_tools::get(*this, name);
}

//----------------------------------------------------------------------------
options& options::setAsString(std::string_view name, const std::string& str)
{
  options_tools::setAsString(*this, name, str);
  return *this;
}

//----------------------------------------------------------------------------
std::string options::getAsString(std::string_view name) const
{
  return options_tools::getAsString(*this, name);
}

//----------------------------------------------------------------------------
options& options::toggle(std::string_view name)
{
  try
  {
    option_variant_t val;
    val = options_tools::get(*this, name);
    options_tools::set(*this, name, !std::get<bool>(val));
    return *this;
  }
  catch (const f3d::options::no_value_exception&)
  {
    options_tools::set(*this, name, true);
    return *this;
  }
  catch (const std::bad_variant_access&)
  {
    throw options::incompatible_exception(
      "Trying to get toggle " + std::string(name) + " with incompatible type");
  }
}

//----------------------------------------------------------------------------
bool options::isSame(const options& other, std::string_view name) const
{
  try
  {
    return options_tools::get(*this, name) == options_tools::get(other, name);
  }
  catch (const f3d::options::no_value_exception&)
  {
    return !this->hasValue(name) && !other.hasValue(name);
  }
}

//----------------------------------------------------------------------------
bool options::hasValue(std::string_view name) const
{
  try
  {
    options_tools::get(*this, name);
    return true;
  }
  catch (const f3d::options::no_value_exception&)
  {
    return false;
  }
}

//----------------------------------------------------------------------------
options& options::copy(const options& from, std::string_view name)
{
  options_tools::set(*this, name, options_tools::get(from, name));
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getAllNames()
{
  return options_tools::getNames();
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getNames() const
{
  const std::vector<std::string> names = options::getAllNames();
  std::vector<std::string> setNames;
  std::copy_if(names.begin(), names.end(), std::back_inserter(setNames),
    [&](const std::string& name) { return this->hasValue(name); });
  return setNames;
}

//----------------------------------------------------------------------------
std::pair<std::string, unsigned int> options::getClosestOption(std::string_view option) const
{
  std::vector<std::string> names = options_tools::getNames();
  if (std::find(names.begin(), names.end(), option) != names.end())
  {
    return { std::string(option), 0 };
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
bool options::isOptional(std::string_view option) const
{
  return options_tools::isOptional(option);
}

//----------------------------------------------------------------------------
options& options::reset(std::string_view name)
{
  options_tools::reset(*this, name);
  return *this;
}

//----------------------------------------------------------------------------
options& options::removeValue(std::string_view name)
{
  if (this->isOptional(name))
  {
    this->reset(name);
  }
  else
  {
    throw options::incompatible_exception("Option " + std::string(name) + " is not not optional");
  }
  return *this;
}

//----------------------------------------------------------------------------
template<typename T>
T options::parse(const std::string& str)
{
  return options_tools::parse<T>(str);
}

//----------------------------------------------------------------------------
template<typename T>
std::string options::format(const T& var)
{
  return options_tools::format(var);
}

//----------------------------------------------------------------------------
#define F3D_DECL_TYPE_INTERNAL(TYPE)                                                               \
  template F3D_EXPORT TYPE options::parse<TYPE>(const std::string& str);                           \
  template F3D_EXPORT std::string options::format<TYPE>(const TYPE& val)
#define F3D_DECL_TYPE(TYPE)                                                                        \
  F3D_DECL_TYPE_INTERNAL(TYPE);                                                                    \
  F3D_DECL_TYPE_INTERNAL(std::vector<TYPE>)
F3D_DECL_TYPE(bool);
F3D_DECL_TYPE(int);
F3D_DECL_TYPE(double);
F3D_DECL_TYPE(f3d::ratio_t);
F3D_DECL_TYPE(std::string);

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
options::no_value_exception::no_value_exception(const std::string& what)
  : exception(what)
{
}
}
