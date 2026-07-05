#include "options_c_api.h"
#include "log.h"
#include "options.h"

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
f3d_options_t* f3d_options_create()
{
  f3d::options* cpp_options = new f3d::options();
  return reinterpret_cast<f3d_options_t*>(cpp_options);
}

//----------------------------------------------------------------------------
void f3d_options_delete(f3d_options_t* options)
{
  if (!options)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  delete cpp_options;
}

//----------------------------------------------------------------------------
void f3d_options_set_as_bool(f3d_options_t* options, const char* name, int value)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->set(name, static_cast<bool>(value));
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_int(f3d_options_t* options, const char* name, int value)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->set(name, value);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_double(f3d_options_t* options, const char* name, double value)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->set(name, value);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_string(f3d_options_t* options, const char* name, const char* value)
{
  if (!options || !name || !value)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->set(name, std::string(value));
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_double_vector(
  f3d_options_t* options, const char* name, const double* values, size_t count)
{
  if (!options || !name || !values)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    std::vector<double> vec(values, values + count);
    cpp_options->set(name, vec);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_int_vector(
  f3d_options_t* options, const char* name, const int* values, size_t count)
{
  if (!options || !name || !values)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    std::vector<int> vec(values, values + count);
    cpp_options->set(name, vec);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
int f3d_options_get_as_bool(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return std::get<bool>(cpp_options->get(name)) ? 1 : 0;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
  catch (const f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
int f3d_options_get_as_int(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return std::get<int>(cpp_options->get(name));
  }
  catch (f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
  catch (f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
double f3d_options_get_as_double(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0.0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return std::get<double>(cpp_options->get(name));
  }
  catch (f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0.0;
  }
  catch (f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0.0;
  }
}

//----------------------------------------------------------------------------
const char* f3d_options_get_as_string(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return nullptr;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    const std::string str = std::get<std::string>(cpp_options->get(name));
    char* result = new char[str.length() + 1];
    std::strcpy(result, str.c_str());
    return result;
  }
  catch (f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
  catch (f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
void f3d_options_get_as_double_vector(
  const f3d_options_t* options, const char* name, double* values, size_t* count)
{
  if (!options || !name || !values || !count)
  {
    return;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    const std::vector<double> vec = std::get<std::vector<double>>(cpp_options->get(name));
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    *count = 0;
    return;
  }
  catch (f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    *count = 0;
    return;
  }
}

//----------------------------------------------------------------------------
void f3d_options_get_as_int_vector(
  const f3d_options_t* options, const char* name, int* values, size_t* count)
{
  if (!options || !name || !values || !count)
  {
    return;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    const std::vector<int> vec = std::get<std::vector<int>>(cpp_options->get(name));
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    *count = 0;
    return;
  }
  catch (f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    *count = 0;
    return;
  }
}

//----------------------------------------------------------------------------
void f3d_options_toggle(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->toggle(name);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
int f3d_options_is_same(const f3d_options_t* options, const f3d_options_t* other, const char* name)
{
  if (!options || !other || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    const f3d::options* cpp_other = reinterpret_cast<const f3d::options*>(other);
    return cpp_options->isSame(*cpp_other, name) ? 1 : 0;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
int f3d_options_has_value(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return cpp_options->hasValue(name) ? 1 : 0;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
void f3d_options_copy(f3d_options_t* options, const f3d_options_t* other, const char* name)
{
  if (!options || !other || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    const f3d::options* cpp_other = reinterpret_cast<const f3d::options*>(other);
    cpp_options->copy(*cpp_other, name);
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
char** f3d_options_get_all_names(size_t* count)
{
  if (!count)
  {
    return nullptr;
  }

  std::vector<std::string> names = f3d::options::getAllNames();
  *count = names.size();

  char** result = new char*[names.size()];
  for (size_t i = 0; i < names.size(); i++)
  {
    result[i] = new char[names[i].length() + 1];
    std::strcpy(result[i], names[i].c_str());
  }

  return result;
}

//----------------------------------------------------------------------------
char** f3d_options_get_names(const f3d_options_t* options, size_t* count)
{
  if (!options || !count)
  {
    return nullptr;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  std::vector<std::string> names = cpp_options->getNames();
  *count = names.size();

  char** result = new char*[names.size()];
  for (size_t i = 0; i < names.size(); i++)
  {
    result[i] = new char[names[i].length() + 1];
    std::strcpy(result[i], names[i].c_str());
  }

  return result;
}

//----------------------------------------------------------------------------
void f3d_options_free_names(char** names, size_t count)
{
  if (!names)
  {
    return;
  }

  for (size_t i = 0; i < count; i++)
  {
    delete[] names[i];
  }
  delete[] names;
}

//----------------------------------------------------------------------------
int f3d_options_is_optional(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return cpp_options->isOptional(name) ? 1 : 0;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
f3d_option_type_t f3d_options_get_type(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return F3D_OPTION_TYPE_INVALID;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    switch (cpp_options->getType(name))
    {
      case f3d::options::option_type::BOOL:
        return F3D_OPTION_TYPE_BOOL;
      case f3d::options::option_type::INT:
        return F3D_OPTION_TYPE_INT;
      case f3d::options::option_type::DOUBLE:
        return F3D_OPTION_TYPE_DOUBLE;
      case f3d::options::option_type::RATIO:
        return F3D_OPTION_TYPE_RATIO;
      case f3d::options::option_type::STRING:
        return F3D_OPTION_TYPE_STRING;
      case f3d::options::option_type::PATH:
        return F3D_OPTION_TYPE_PATH;
      case f3d::options::option_type::COLOR:
        return F3D_OPTION_TYPE_COLOR;
      case f3d::options::option_type::DIRECTION:
        return F3D_OPTION_TYPE_DIRECTION;
      case f3d::options::option_type::COLORMAP:
        return F3D_OPTION_TYPE_COLORMAP;
      case f3d::options::option_type::TRANSFORM2D:
        return F3D_OPTION_TYPE_TRANSFORM2D;
      case f3d::options::option_type::DOUBLE_VECTOR:
        return F3D_OPTION_TYPE_DOUBLE_VECTOR;
      case f3d::options::option_type::INT_VECTOR:
        return F3D_OPTION_TYPE_INT_VECTOR;
      default:
        // Unreachable
        assert(false);
        return F3D_OPTION_TYPE_INVALID;
    }
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return F3D_OPTION_TYPE_INVALID;
  }
}

//----------------------------------------------------------------------------
void f3d_options_reset(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->reset(name);
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_remove_value(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->removeValue(name);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
int f3d_options_has_domain(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    return cpp_options->hasDomain(name) ? 1 : 0;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
f3d_domain_style_t f3d_options_get_domain_style(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return F3D_DOMAIN_STYLE_NONE;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    f3d::options::domain_style ds = cpp_options->getDomainStyle(name);
    switch (ds)
    {
      case f3d::options::domain_style::RANGE:
        return F3D_DOMAIN_STYLE_RANGE;
      case f3d::options::domain_style::ENUM:
        return F3D_DOMAIN_STYLE_ENUM;
      default:
      case f3d::options::domain_style::INDEX:
        return F3D_DOMAIN_STYLE_INDEX;
    }
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
    return F3D_DOMAIN_STYLE_NONE;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return F3D_DOMAIN_STYLE_NONE;
  }
}

//----------------------------------------------------------------------------
char** f3d_options_get_enum_domain(const f3d_options_t* options, const char* name, int* count)
{
  if (!options || !name || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  try
  {
    const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
    std::vector<std::string> enumeration = cpp_options->getEnumDomain(name);

    *count = static_cast<int>(enumeration.size());
    if (enumeration.empty())
    {
      return nullptr;
    }

    char** result = new char*[enumeration.size()];

    for (size_t i = 0; i < enumeration.size(); ++i)
    {
      result[i] = new char[enumeration[i].length() + 1];
      std::strcpy(result[i], enumeration[i].c_str());
    }

    return result;
  }

  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
void f3d_options_increase(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->increase(name);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_decrease(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->decrease(name);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_cycle(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  try
  {
    f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
    cpp_options->cycle(name);
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
const char* f3d_options_get_as_string_representation(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return nullptr;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  try
  {
    std::string str = cpp_options->getAsString(name);
    char* result = new char[str.length() + 1];
    std::strcpy(result, str.c_str());
    return result;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
  catch (const f3d::options::no_value_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_string_representation(
  f3d_options_t* options, const char* name, const char* str)
{
  if (!options || !name || !str)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  try
  {
    cpp_options->setAsString(name, str);
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    f3d::log::error(ex.what());
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_free_string(const char* str)
{
  if (!str)
  {
    return;
  }

  delete[] str;
}

//----------------------------------------------------------------------------
void f3d_options_free_string_array(char** array, int count)
{
  if (!array)
  {
    return;
  }

  for (int i = 0; i < count; ++i)
  {
    delete[] array[i];
  }
  delete[] array;
}

//----------------------------------------------------------------------------
void f3d_options_get_closest_option(
  const f3d_options_t* options, const char* option, char** closest, unsigned int* distance)
{
  if (!options || !option || !closest || !distance)
  {
    return;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  auto result = cpp_options->getClosestOption(option);

  char* result_str = new char[result.first.length() + 1];
  std::strcpy(result_str, result.first.c_str());
  *closest = result_str;
  *distance = result.second;
}

//----------------------------------------------------------------------------
int f3d_options_parse_bool(const char* str)
{
  if (!str)
  {
    return 0;
  }

  try
  {
    return f3d::options::parse<bool>(str) ? 1 : 0;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
int f3d_options_parse_int(const char* str)
{
  if (!str)
  {
    return 0;
  }

  try
  {
    return f3d::options::parse<int>(str);
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0;
  }
}

//----------------------------------------------------------------------------
double f3d_options_parse_double(const char* str)
{
  if (!str)
  {
    return 0.0;
  }

  try
  {
    return f3d::options::parse<double>(str);
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return 0.0;
  }
}

//----------------------------------------------------------------------------
const char* f3d_options_parse_string(const char* str)
{
  if (!str)
  {
    return nullptr;
  }

  try
  {
    std::string result = f3d::options::parse<std::string>(str);
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
void f3d_options_parse_double_vector(const char* str, double* values, size_t* count)
{
  if (!str || !values || !count)
  {
    return;
  }

  try
  {
    std::vector<double> vec = f3d::options::parse<std::vector<double>>(str);
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return;
  }
}

//----------------------------------------------------------------------------
void f3d_options_parse_int_vector(const char* str, int* values, size_t* count)
{
  if (!str || !values || !count)
  {
    return;
  }

  try
  {
    std::vector<int> vec = f3d::options::parse<std::vector<int>>(str);
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    f3d::log::error(ex.what());
    return;
  }
}

//----------------------------------------------------------------------------
const char* f3d_options_format_bool(int value)
{
  std::string result = f3d::options::format(static_cast<bool>(value));
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_int(int value)
{
  std::string result = f3d::options::format(value);
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_double(double value)
{
  std::string result = f3d::options::format(value);
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_string(const char* value)
{
  if (!value)
  {
    return nullptr;
  }

  std::string result = f3d::options::format(std::string(value));
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_double_vector(const double* values, size_t count)
{
  if (!values)
  {
    return nullptr;
  }

  std::vector<double> vec(values, values + count);
  std::string result = f3d::options::format(vec);
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_int_vector(const int* values, size_t count)
{
  if (!values)
  {
    return nullptr;
  }

  std::vector<int> vec(values, values + count);
  std::string result = f3d::options::format(vec);
  char* result_str = new char[result.length() + 1];
  std::strcpy(result_str, result.c_str());
  return result_str;
}
