#include "animation_c_api.h"

#include "animation.h"

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
