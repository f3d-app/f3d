#include "options_c_api.h"
#include "options.h"

#include <cstring>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
void f3d_options_set_as_bool(f3d_options_t* options, const char* name, int value)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->set(name, static_cast<bool>(value));
}

//----------------------------------------------------------------------------
void f3d_options_set_as_int(f3d_options_t* options, const char* name, int value)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->set(name, value);
}

//----------------------------------------------------------------------------
void f3d_options_set_as_double(f3d_options_t* options, const char* name, double value)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->set(name, value);
}

//----------------------------------------------------------------------------
void f3d_options_set_as_string(f3d_options_t* options, const char* name, const char* value)
{
  if (!options || !name || !value)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->set(name, std::string(value));
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
  cpp_options->set(name, vec);
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
  cpp_options->set(name, vec);
}

//----------------------------------------------------------------------------
int f3d_options_get_as_bool(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  return std::get<bool>(cpp_options->get(name)) ? 1 : 0;
}

//----------------------------------------------------------------------------
int f3d_options_get_as_int(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  return std::get<int>(cpp_options->get(name));
}

//----------------------------------------------------------------------------
double f3d_options_get_as_double(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0.0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  return std::get<double>(cpp_options->get(name));
}

//----------------------------------------------------------------------------
const char* f3d_options_get_as_string(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return nullptr;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  const std::string str = std::get<std::string>(cpp_options->get(name));
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
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
  const std::vector<double> vec = std::get<std::vector<double>>(cpp_options->get(name));
  *count = vec.size();
  std::copy(vec.begin(), vec.end(), values);
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
  const std::vector<int> vec = std::get<std::vector<int>>(cpp_options->get(name));
  *count = vec.size();
  std::copy(vec.begin(), vec.end(), values);
}

//----------------------------------------------------------------------------
void f3d_options_toggle(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->toggle(name);
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
  return cpp_options->isSame(*cpp_other, name) ? 1 : 0;
}

//----------------------------------------------------------------------------
int f3d_options_has_value(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return 0;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  return cpp_options->hasValue(name) ? 1 : 0;
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
  cpp_options->copy(*cpp_other, name);
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

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  return cpp_options->isOptional(name) ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_options_reset(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->reset(name);
}

//----------------------------------------------------------------------------
void f3d_options_remove_value(f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return;
  }

  f3d::options* cpp_options = reinterpret_cast<f3d::options*>(options);
  cpp_options->removeValue(name);
}

//----------------------------------------------------------------------------
const char* f3d_options_get_as_string_representation(const f3d_options_t* options, const char* name)
{
  if (!options || !name)
  {
    return nullptr;
  }

  const f3d::options* cpp_options = reinterpret_cast<const f3d::options*>(options);
  std::string str = cpp_options->getAsString(name);
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
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
  cpp_options->setAsString(name, str);
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
