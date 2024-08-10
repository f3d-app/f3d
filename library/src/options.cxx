#include "options.h"
#include "options_tools.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include "vtkF3DConfigure.h"

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

#ifdef F3D_ENABLE_CXX17_API
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
#endif

//----------------------------------------------------------------------------
options& options::setAsString(const std::string& name, const std::string& str)
{
  options_tools::setAsString(*this, name, str);
  return *this;
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name) const
{
  return options_tools::getAsString(*this, name);
}

//----------------------------------------------------------------------------
options& options::toggle(const std::string& name)
{
  try
  {
    option_variant_t val;
    val = options_tools::get(*this, name);
    options_tools::set(*this, name, !std::get<bool>(val));
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
