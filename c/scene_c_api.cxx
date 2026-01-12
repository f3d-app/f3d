#include "scene_c_api.h"
#include "scene.h"
#include "types.h"

#include <filesystem>
#include <log.h>
#include <vector>

namespace
{
//----------------------------------------------------------------------------
f3d::light_state_t to_cpp_light_state(const f3d_light_state_t* c_state)
{
  f3d::light_state_t cpp_state;

  switch (c_state->type)
  {
    case F3D_LIGHT_TYPE_HEADLIGHT:
      cpp_state.type = f3d::light_type::HEADLIGHT;
      break;
    case F3D_LIGHT_TYPE_CAMERA_LIGHT:
      cpp_state.type = f3d::light_type::CAMERA_LIGHT;
      break;
    case F3D_LIGHT_TYPE_SCENE_LIGHT:
      cpp_state.type = f3d::light_type::SCENE_LIGHT;
      break;
  }

  cpp_state.position[0] = c_state->position[0];
  cpp_state.position[1] = c_state->position[1];
  cpp_state.position[2] = c_state->position[2];

  cpp_state.color[0] = c_state->color.data[0];
  cpp_state.color[1] = c_state->color.data[1];
  cpp_state.color[2] = c_state->color.data[2];

  cpp_state.direction[0] = c_state->direction[0];
  cpp_state.direction[1] = c_state->direction[1];
  cpp_state.direction[2] = c_state->direction[2];

  cpp_state.positionalLight = c_state->positional_light != 0;
  cpp_state.intensity = c_state->intensity;
  cpp_state.switchState = c_state->switch_state != 0;

  return cpp_state;
}

//----------------------------------------------------------------------------
f3d_light_state_t* to_c_light_state(const f3d::light_state_t& cpp_state)
{
  f3d_light_state_t* c_state = new f3d_light_state_t;

  switch (cpp_state.type)
  {
    case f3d::light_type::HEADLIGHT:
      c_state->type = F3D_LIGHT_TYPE_HEADLIGHT;
      break;
    case f3d::light_type::CAMERA_LIGHT:
      c_state->type = F3D_LIGHT_TYPE_CAMERA_LIGHT;
      break;
    case f3d::light_type::SCENE_LIGHT:
      c_state->type = F3D_LIGHT_TYPE_SCENE_LIGHT;
      break;
  }

  c_state->position[0] = cpp_state.position[0];
  c_state->position[1] = cpp_state.position[1];
  c_state->position[2] = cpp_state.position[2];

  c_state->color.data[0] = cpp_state.color[0];
  c_state->color.data[1] = cpp_state.color[1];
  c_state->color.data[2] = cpp_state.color[2];

  c_state->direction[0] = cpp_state.direction[0];
  c_state->direction[1] = cpp_state.direction[1];
  c_state->direction[2] = cpp_state.direction[2];

  c_state->positional_light = cpp_state.positionalLight ? 1 : 0;
  c_state->intensity = cpp_state.intensity;
  c_state->switch_state = cpp_state.switchState ? 1 : 0;

  return c_state;
}

//----------------------------------------------------------------------------
f3d::mesh_t to_cpp_mesh(const f3d_mesh_t* c_mesh)
{
  f3d::mesh_t cpp_mesh;

  if (c_mesh->points && c_mesh->points_count > 0)
  {
    cpp_mesh.points.assign(c_mesh->points, c_mesh->points + c_mesh->points_count);
  }

  if (c_mesh->normals && c_mesh->normals_count > 0)
  {
    cpp_mesh.normals.assign(c_mesh->normals, c_mesh->normals + c_mesh->normals_count);
  }

  if (c_mesh->texture_coordinates && c_mesh->texture_coordinates_count > 0)
  {
    cpp_mesh.texture_coordinates.assign(
      c_mesh->texture_coordinates, c_mesh->texture_coordinates + c_mesh->texture_coordinates_count);
  }

  if (c_mesh->face_sides && c_mesh->face_sides_count > 0)
  {
    cpp_mesh.face_sides.assign(c_mesh->face_sides, c_mesh->face_sides + c_mesh->face_sides_count);
  }

  if (c_mesh->face_indices && c_mesh->face_indices_count > 0)
  {
    cpp_mesh.face_indices.assign(
      c_mesh->face_indices, c_mesh->face_indices + c_mesh->face_indices_count);
  }

  return cpp_mesh;
}
}

//----------------------------------------------------------------------------
int f3d_scene_supports(f3d_scene_t* scene, const char* file_path)
{
  if (!scene || !file_path)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  return cpp_scene->supports(std::filesystem::path(file_path)) ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_scene_clear(f3d_scene_t* scene)
{
  if (!scene)
  {
    return;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  cpp_scene->clear();
}

//----------------------------------------------------------------------------
int f3d_scene_add(f3d_scene_t* scene, const char* file_path)
{
  if (!scene || !file_path)
  {
    return 0;
  }

  try
  {
    f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
    cpp_scene->add(std::filesystem::path(file_path));
  }
  catch (const f3d::scene::load_failure_exception& e)
  {
    f3d::log::error("Failed to add file to scene: {}", file_path);
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int f3d_scene_add_multiple(f3d_scene_t* scene, const char** file_paths, size_t count)
{
  if (!scene || !file_paths)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  std::vector<std::filesystem::path> paths;
  paths.reserve(count);

  for (size_t i = 0; i < count; ++i)
  {
    if (file_paths[i])
    {
      paths.emplace_back(file_paths[i]);
    }
  }

  try
  {
    cpp_scene->add(paths);
  }
  catch (const f3d::scene::load_failure_exception& e)
  {
    f3d::log::error("Failed to add multiple files to scene");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int f3d_scene_add_mesh(f3d_scene_t* scene, const f3d_mesh_t* mesh)
{
  if (!scene || !mesh)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  f3d::mesh_t cpp_mesh = to_cpp_mesh(mesh);

  try
  {
    cpp_scene->add(cpp_mesh);
  }
  catch (const f3d::scene::load_failure_exception& e)
  {
    f3d::log::error("Failed to add mesh to scene");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int f3d_scene_add_buffer(f3d_scene_t* scene, void* buffer, size_t size)
{
  if (!scene || !buffer || size == 0)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);

  try
  {
    cpp_scene->add(reinterpret_cast<std::byte*>(buffer), size);
  }
  catch (const f3d::scene::load_failure_exception& e)
  {
    f3d::log::error("Failed to add buffer to scene");
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
void f3d_scene_load_animation_time(f3d_scene_t* scene, double time_value)
{
  if (!scene)
  {
    return;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  cpp_scene->loadAnimationTime(time_value);
}

//----------------------------------------------------------------------------
double* f3d_scene_animation_keyframes(f3d_scene_t* scene, unsigned int* count)
{
  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  std::vector<double> keyframes = cpp_scene->animationKeyFrames();
  *count = keyframes.size();
  double* times = new double[keyframes.size()];
  for (size_t i = 0; i < keyframes.size(); ++i)
  {
    times[i] = keyframes[i];
  }
  return times;
}

//----------------------------------------------------------------------------
void f3d_scene_animation_time_range(f3d_scene_t* scene, double* min_time, double* max_time)
{
  if (!scene)
  {
    return;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  auto range = cpp_scene->animationTimeRange();

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
unsigned int f3d_scene_available_animations(const f3d_scene_t* scene)
{
  if (!scene)
  {
    return 0;
  }

  const f3d::scene* cpp_scene = reinterpret_cast<const f3d::scene*>(scene);
  return cpp_scene->availableAnimations();
}

//----------------------------------------------------------------------------
int f3d_scene_add_light(const f3d_scene_t* scene, const f3d_light_state_t* light_state)
{
  if (!scene || !light_state)
  {
    return -1;
  }

  const f3d::scene* cpp_scene = reinterpret_cast<const f3d::scene*>(scene);
  f3d::light_state_t cpp_state = to_cpp_light_state(light_state);
  return cpp_scene->addLight(cpp_state);
}

//----------------------------------------------------------------------------
int f3d_scene_remove_light(f3d_scene_t* scene, int index)
{
  if (!scene)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);

  try
  {
    cpp_scene->removeLight(index);
  }
  catch (const f3d::scene::light_exception& e)
  {
    f3d::log::error("Failed to remove light at index {}: {}", index, e.what());
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
int f3d_scene_update_light(f3d_scene_t* scene, int index, const f3d_light_state_t* light_state)
{
  if (!scene || !light_state)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  f3d::light_state_t cpp_state = to_cpp_light_state(light_state);

  try
  {
    cpp_scene->updateLight(index, cpp_state);
  }
  catch (const f3d::scene::light_exception& e)
  {
    f3d::log::error("Failed to update light at index {}: {}", index, e.what());
    return 0;
  }

  return 1;
}

//----------------------------------------------------------------------------
f3d_light_state_t* f3d_scene_get_light(const f3d_scene_t* scene, int index)
{
  if (!scene)
  {
    return nullptr;
  }

  const f3d::scene* cpp_scene = reinterpret_cast<const f3d::scene*>(scene);

  try
  {
    f3d::light_state_t cpp_state = cpp_scene->getLight(index);
    return to_c_light_state(cpp_state);
  }
  catch (const f3d::scene::light_exception& e)
  {
    f3d::log::error("Failed to get light at index {}: {}", index, e.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
int f3d_scene_get_light_count(const f3d_scene_t* scene)
{
  if (!scene)
  {
    return 0;
  }

  const f3d::scene* cpp_scene = reinterpret_cast<const f3d::scene*>(scene);
  return cpp_scene->getLightCount();
}

//----------------------------------------------------------------------------
void f3d_scene_remove_all_lights(f3d_scene_t* scene)
{
  if (!scene)
  {
    return;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  cpp_scene->removeAllLights();
}
