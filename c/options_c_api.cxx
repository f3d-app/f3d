#include "options_c_api.h"
#include "options.h"
#include "log.h"
#include <cstring>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
f3d_options_t* f3d_options_create()
{
  try
  {
    f3d::options* cpp_options = new f3d::options();
    return reinterpret_cast<f3d_options_t*>(cpp_options);
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for f3d_options_t: ", e.what());
  }
  
  return nullptr;
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

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->set(name, static_cast<bool>(value));
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_int(f3d_options_t* options, const char* name, int value)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->set(name, value);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_double(f3d_options_t* options, const char* name, double value)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->set(name, value);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_set_as_string(f3d_options_t* options, const char* name, const char* value)
{
  if (!options || !name || !value)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->set(name, std::string(value));
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
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

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  std::vector<double> vec(values, values + count);

  try
  {
    cpp_options->set(name, vec);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
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

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  std::vector<int> vec(values, values + count);

  try
  {
    cpp_options->set(name, vec);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
int f3d_options_get_as_bool(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    return std::get<bool>(cpp_options->get(name)) ? 1 : 0;
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }

  return 0;
}

//----------------------------------------------------------------------------
int f3d_options_get_as_int(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    return std::get<int>(cpp_options->get(name));
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }

  return 0;
}

//----------------------------------------------------------------------------
double f3d_options_get_as_double(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0.0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    return std::get<double>(cpp_options->get(name));
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }

  return 0.0;
}

//----------------------------------------------------------------------------
const char* f3d_options_get_as_string(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return nullptr;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    const std::string str = std::get<std::string>(cpp_options->get(name));
    char* result = new char[str.length() + 1];
    std::strcpy(result, str.c_str());
    return result;
  }
  catch (const f3d::options::no_value_exception& e)
  {
    f3d::log::warn(e.what());
  }
  catch (const f3d::options::inexistent_exception& e)
  {
    f3d::log::warn(e.what());
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char[]: ", e.what());
  }
  
  return nullptr;
}

//----------------------------------------------------------------------------
void f3d_options_get_as_double_vector(
  const f3d_options_t* options, const char* name, double* values, size_t* count)
{
  if (!options || !name || !values || !count)
  {
    return;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    const std::vector<double> vec = std::get<std::vector<double>>(cpp_options->get(name));
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
    *count = 0;
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

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    const std::vector<int> vec = std::get<std::vector<int>>(cpp_options->get(name));
    *count = vec.size();
    std::copy(vec.begin(), vec.end(), values);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
    *count = 0;
  }
}

//----------------------------------------------------------------------------
void f3d_options_toggle(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->toggle(name);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
int f3d_options_is_same(const f3d_options_t* options, const f3d_options_t* other, const char* name)
{
  if (!options || !other || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  const f3d::options* cpp_other = reinterpret_cast<const f3d::options*>(other);

  try
  {
    return cpp_options->isSame(*cpp_other, name) ? 1 : 0;
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }

  return 0;
}

//----------------------------------------------------------------------------
int f3d_options_has_value(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    return cpp_options->hasValue(name) ? 1 : 0;
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
  
  return 0;
}

//----------------------------------------------------------------------------
void f3d_options_copy(f3d_options_t* options, const f3d_options_t* other, const char* name)
{
  if (!options || !other || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  const f3d::options* cpp_other = reinterpret_cast<const f3d::options*>(other);

  try
  {
    cpp_options->copy(*cpp_other, name);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
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

  try
  {
    char** result = new char*[names.size()];
    for (size_t i = 0; i < names.size(); i++)
    {
      result[i] = new char[names[i].length() + 1];
      std::strcpy(result[i], names[i].c_str());
    }

    return result;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char**: ", e.what());
  }

  return nullptr;
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

  try
  {
    char** result = new char*[names.size()];
    for (size_t i = 0; i < names.size(); i++)
    {
      result[i] = new char[names[i].length() + 1];
      std::strcpy(result[i], names[i].c_str());
    }

    return result;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char**: ", e.what());
  }
  
  return nullptr;
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

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);

  try
  {
    return cpp_options->isOptional(name) ? 1 : 0;
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }

  return 0;
}

//----------------------------------------------------------------------------
void f3d_options_reset(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->reset(name);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
  }
}

//----------------------------------------------------------------------------
void f3d_options_remove_value(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);

  try
  {
    cpp_options->removeValue(name);
  }
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
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
  catch (const f3d::options::no_value_exception& e)
  {
    f3d::log::warn(e.what());
  }
  catch (const f3d::options::inexistent_exception& e)
  {
    f3d::log::warn(e.what());
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }
  
  return nullptr;
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
  catch (const std::exception& e)
  {
    f3d::log::warn(e.what());
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
void f3d_options_get_closest_option(
  const f3d_options_t* options, const char* option, char** closest, unsigned int* distance)
{
  if (!options || !option || !closest || !distance)
  {
    return;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  auto result = cpp_options->getClosestOption(option);

  try
  {
    char* result_str = new char[result.first.length() + 1];
    std::strcpy(result_str, result.first.c_str());
    *closest = result_str;
    *distance = result.second;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
    *closest = nullptr;
    *distance = 0U;
  }
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
  catch (const std::exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
  }

  return 0;
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
  catch (const std::exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
  }

  return 0;
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
  catch (const std::exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
  }

  return 0.0;
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
  catch (const f3d::options::parsing_exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
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
  catch (const std::exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
    *count = 0U;
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
  catch (const std::exception& e)
  {
    f3d::log::error("Parsing failed: ", e.what());
  }
}

//----------------------------------------------------------------------------
const char* f3d_options_format_bool(int value)
{
  std::string result = f3d::options::format(static_cast<bool>(value));

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_int(int value)
{
  std::string result = f3d::options::format(value);

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_double(double value)
{
  std::string result = f3d::options::format(value);

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
}

//----------------------------------------------------------------------------
const char* f3d_options_format_string(const char* value)
{
  if (!value)
  {
    return nullptr;
  }

  std::string result = f3d::options::format(std::string(value));

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
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

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
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

  try
  {
    char* result_str = new char[result.length() + 1];
    std::strcpy(result_str, result.c_str());
    return result_str;
  }
  catch (const std::bad_alloc& e)
  {
    f3d::log::error("Failed to allocate memory for char*: ", e.what());
  }

  return nullptr;
}
