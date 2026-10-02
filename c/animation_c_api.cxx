#include "animation_c_api.h"

#include "animation.h"

#include <cstring>

//----------------------------------------------------------------------------
void f3d_animation_load_time(f3d_animation_t* animation, double time_value)
{
  if (!animation)
  {
    return;
  }

  f3d::animation* cpp_animation = reinterpret_cast<f3d::animation*>(animation);
  cpp_animation->loadTime(time_value);
}

//----------------------------------------------------------------------------
double* f3d_animation_keyframes(f3d_animation_t* animation, unsigned int* count)
{
  f3d::animation* cpp_animation = reinterpret_cast<f3d::animation*>(animation);
  std::vector<double> keyframes = cpp_animation->keyFrames();
  *count = keyframes.size();
  double* times = new double[keyframes.size()];
  for (size_t i = 0; i < keyframes.size(); ++i)
  {
    times[i] = keyframes[i];
  }
  return times;
}

//----------------------------------------------------------------------------
void f3d_animation_destroy_keyframes(double* keyframes)
{
  delete[] keyframes;
}

//----------------------------------------------------------------------------
void f3d_animation_time_range(f3d_animation_t* animation, double* min_time, double* max_time)
{
  if (!animation)
  {
    return;
  }

  f3d::animation* cpp_animation = reinterpret_cast<f3d::animation*>(animation);
  auto range = cpp_animation->timeRange();

  if (min_time)
  {
    *min_time = range.first;
  }
  if (max_time)
  {
    *max_time = range.second;
  }
}

//----------------------------------------------------------------------------
unsigned int f3d_animation_count(const f3d_animation_t* animation)
{
  if (!animation)
  {
    return 0;
  }

  const f3d::animation* cpp_animation = reinterpret_cast<const f3d::animation*>(animation);
  return cpp_animation->count();
}

//----------------------------------------------------------------------------
char* f3d_animation_get_name(const f3d_animation_t* animation, int index)
{
  if (!animation)
  {
    return nullptr;
  }

  const f3d::animation* cpp_animation = reinterpret_cast<const f3d::animation*>(animation);
  const std::string str = cpp_animation->getName(index);
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
}

//----------------------------------------------------------------------------
char** f3d_animation_get_names(const f3d_animation_t* animation, int* count)
{
  if (!animation || !count)
  {
    if (count)
    {
      *count = 0;
    }
    return nullptr;
  }

  const f3d::animation* cpp_animation = reinterpret_cast<const f3d::animation*>(animation);
  std::vector<std::string> names = cpp_animation->getNames();

  *count = static_cast<int>(names.size());
  if (names.empty())
  {
    return nullptr;
  }

  char** result = new char*[names.size()];

  for (size_t i = 0; i < names.size(); ++i)
  {
    result[i] = new char[names[i].length() + 1];
    std::strcpy(result[i], names[i].c_str());
  }

  return result;
}

//----------------------------------------------------------------------------
void f3d_animation_destroy_string(const char* str)
{
  delete[] str;
}

//----------------------------------------------------------------------------
void f3d_animation_destroy_string_array(char** array, int count)
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
