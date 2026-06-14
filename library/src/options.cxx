#include "macros.h"

F3D_SILENT_WARNING_PUSH()
F3D_SILENT_WARNING_DECL(4996, "deprecated-declarations")
#include "options.h"
F3D_SILENT_WARNING_POP()

#include "options_generated.h"
#include "options_tools.h"

#include "export.h"
#include "init.h"
#include "log.h"
#include "utils.h"

#include <algorithm>
#include <limits>
#include <string>

namespace
{

template<typename T> void increase(T& val, const f3d::options::domain_range_t<T>& domain, bool up)
{
  char dir = up ? +1 : -1;
  T newVal = val;

  // TODO how to handle numeric limits properly ?
  newVal += dir * domain.increment;

  // TODO this can be incorrect in case of double computation, how to adress ?
  if ((up && newVal <= domain.range[1]) || (!up && newVal >= domain.range[0]))  
  {
    val = newVal;
  }
}

template<typename T> void increase(std::vector<T>& vec, const f3d::options::domain_range_t<T>& domain, bool up)
{
  std::ranges::for_each(vec, [domain, up](T& val){::increase(val, domain, up);});
}

template<typename T>void increase(std::optional<T>& val, const f3d::options::domain_range_t<T>& domain, bool up)
{
  if (!val.has_value())
  {
    if (domain.range[0] != domain.range[1])
    {
      val = up ? domain.range[0] : domain.range[1];
    }
  }
  else
  {
    ::increase(val.value(), domain, up);
  }
}

// Implicit double domain
void increase(double& val, bool up)
{
  f3d::options::domain_range_t domain {{-std::numeric_limits<double>::max(), std::numeric_limits<double>::max()}, 0.1};
  ::increase(val, domain, up);
}

// Implicit int domain
void increase(int& val, int up)
{
  f3d::options::domain_range_t domain {{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}, 1};
  ::increase(val, domain, up);
}

// Implicit f3d::ratio_t domain
void increase(f3d::ratio_t& val, bool up)
{
  f3d::options::domain_range_t domain {{f3d::ratio_t(0.), f3d::ratio_t(1.)}, f3d::ratio_t(0.1)};
  ::increase(val, domain, up);
}

// Implicit optional<double> domain
void increase(std::optional<double>& val, bool up)
{
  if (!val.has_value())
  {
    val = 1.0;
  }
  else
  {
    ::increase(val.value(), up);
  }
}

// Implicit optional<int> domain
void increase(std::optional<int>& val, bool up)
{
  if (!val.has_value())
  {
    val = 0;
  }
  else
  {
    ::increase(val.value(), up);
  }
}

template<typename T> void cycle(T& val, const f3d::options::domain_enum_t<T>& domain)
{
  auto it = std::ranges::find(domain.enumeration, val);
  if (it != domain.enumeration.end())
  {
    it++;
    if(it == domain.enumeration.end())
    {
      it = domain.enumeration.begin();
    }
    val = *it;
  }
  else
  {
    val = domain.enumeration.front();
  }
}

void increase(f3d::options& opt, std::string_view name, bool up)
{
  // manual handle certains option for now
  if (name == "render.light.intensity")
  {
    ::increase(opt.render.light.intensity, opt.domains.render.light.intensity, up);
  }
  else if (name == "ui.backdrop.opacity")
  {
    ::increase(opt.ui.backdrop.opacity, up);
  }
  else if (name == "render.grid.reflection")
  {
    ::increase(opt.render.grid.reflection, up);
  }
  else if (name == "scene.animation.speed_factor")
  {
    ::increase(opt.scene.animation.speed_factor, opt.domains.scene.animation.speed_factor, up);
  }
  else if (name == "render.line_width")
  {
    ::increase(opt.render.line_width, up);
  }
  else if (name == "scene.camera.index")
  {
    ::increase(opt.scene.camera.index, opt.domains.scene.camera.index, up);
  }
  else if (name == "scene.animation.indices")
  {
    ::increase(opt.scene.animation.indices, opt.domains.scene.animation.indices, up);
  }
}

void cycle(f3d::options& opt, std::string_view name)
{
  if (name == "render.effect.blending.mode")
  {
    ::cycle(opt.render.effect.blending.mode, opt.domains.render.effect.blending.mode);
  }
}
}

namespace f3d
{
//----------------------------------------------------------------------------
options::options()
{
  detail::init::initialize();
}

//----------------------------------------------------------------------------
options::~options() = default;

//----------------------------------------------------------------------------
options::options(const options& opt) = default;

//----------------------------------------------------------------------------
options& options::operator=(const options& opt) = default;

//----------------------------------------------------------------------------
options::options(options&& other) noexcept = default;

//----------------------------------------------------------------------------
options& options::operator=(options&& other) noexcept = default;

//----------------------------------------------------------------------------
options& options::set(std::string_view name, const option_variant_t& value)
{
  options_generated::set(*this, name, value);
  return *this;
}

//----------------------------------------------------------------------------
option_variant_t options::get(std::string_view name) const
{
  return options_generated::get(*this, name);
}

//----------------------------------------------------------------------------
options& options::setAsString(std::string_view name, const std::string& str)
{
  options_generated::setAsString(*this, name, str);
  return *this;
}

//----------------------------------------------------------------------------
std::string options::getAsString(std::string_view name) const
{
  return options_generated::getAsString(*this, name);
}

//----------------------------------------------------------------------------
options& options::toggle(std::string_view name)
{
  try
  {
    option_variant_t val;
    val = options_generated::get(*this, name);
    options_generated::set(*this, name, !std::get<bool>(val));
    return *this;
  }
  catch (const f3d::options::no_value_exception&)
  {
    options_generated::set(*this, name, true);
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
    return options_generated::get(*this, name) == options_generated::get(other, name);
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
    options_generated::get(*this, name);
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
  options_generated::set(*this, name, options_generated::get(from, name));
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> options::getAllNames()
{
  return options_generated::getNames();
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
  std::vector<std::string> names = options_generated::getNames();
  if (std::ranges::find(names, option) != names.end())
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
  return options_generated::isOptional(option);
}

//----------------------------------------------------------------------------
options& options::reset(std::string_view name)
{
  options_generated::reset(*this, name);
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
options& options::increase(std::string_view name)
{
  //  options_generated::increase(*this, name, true);
  ::increase(*this, name, true);

  return *this;
}

//----------------------------------------------------------------------------
options& options::decrease(std::string_view name)
{
  //  options_generated::increase(*this, name, false);
  ::increase(*this, name, false);

  return *this;
}

//----------------------------------------------------------------------------
options& options::cycle(std::string_view name)
{
  //  options_generated::cycle(*this, name);
  ::cycle(*this, name);

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
#define F3D_DECL_TYPE(TYPE)                                                                        \
  template F3D_EXPORT TYPE options::parse<TYPE>(const std::string& str);                           \
  template F3D_EXPORT std::string options::format<TYPE>(const TYPE& val)
#define F3D_DECL_TYPE_WITH_VEC(TYPE)                                                               \
  F3D_DECL_TYPE(TYPE);                                                                             \
  F3D_DECL_TYPE(std::vector<TYPE>)
F3D_DECL_TYPE_WITH_VEC(bool);
F3D_DECL_TYPE_WITH_VEC(int);
F3D_DECL_TYPE_WITH_VEC(double);
F3D_DECL_TYPE_WITH_VEC(f3d::ratio_t);
F3D_DECL_TYPE_WITH_VEC(std::string);
F3D_DECL_TYPE(color_t);
F3D_DECL_TYPE(direction_t);
F3D_DECL_TYPE(colormap_t);
F3D_DECL_TYPE(transform2d_t);
F3D_DECL_TYPE(std::filesystem::path);

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
