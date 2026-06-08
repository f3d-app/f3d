#include "scene_c_api.h"
#include "mesh_view.h"
#include "scene.h"
#include "types.h"

#include <array>
#include <filesystem>
#include <log.h>
#include <memory>
#include <string>
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

//----------------------------------------------------------------------------
f3d::mesh_view::data_array_t to_cpp_data_array(const f3d_data_array_t& c)
{
  f3d::mesh_view::data_array_t a;
  if (c.name)
  {
    a.name = c.name;
  }
  a.type = static_cast<f3d::mesh_view::data_type>(c.type);
  a.data = c.data;
  a.components = c.components ? c.components : 1;
  a.stride = c.stride ? c.stride : 1;
  a.timeDependent = c.time_dependent != 0;
  return a;
}

//----------------------------------------------------------------------------
f3d::mesh_view::cell_array_t to_cpp_cell_array(const f3d_cell_array_t& c)
{
  f3d::mesh_view::cell_array_t a;
  // 0 from a zero-initialized C struct means "no cell": map it to the C++ default of 1.
  a.offsetCount = c.offset_count ? c.offset_count : 1;
  a.offsets = to_cpp_data_array(c.offsets);
  a.indexCount = c.index_count;
  a.indices = to_cpp_data_array(c.indices);
  return a;
}

//----------------------------------------------------------------------------
f3d::mesh_view::memory_view_t to_cpp_memory_view(const f3d_memory_view_t* c)
{
  f3d::mesh_view::memory_view_t v;
  v.pointCount = c->point_count;
  v.points = to_cpp_data_array(c->points);
  v.normals = to_cpp_data_array(c->normals);
  v.textureCoordinates = to_cpp_data_array(c->texture_coordinates);
  v.vertices = to_cpp_cell_array(c->vertices);
  v.lines = to_cpp_cell_array(c->lines);
  v.polygons = to_cpp_cell_array(c->polygons);

  v.pointScalars.reserve(c->point_scalars_count);
  for (size_t i = 0; i < c->point_scalars_count; ++i)
  {
    v.pointScalars.push_back(to_cpp_data_array(c->point_scalars[i]));
  }

  v.cellScalars.reserve(c->cell_scalars_count);
  for (size_t i = 0; i < c->cell_scalars_count; ++i)
  {
    v.cellScalars.push_back(to_cpp_data_array(c->cell_scalars[i]));
  }

  v.baseColorTexture.data = c->base_color_texture;
  v.baseColorTexture.width = c->base_color_texture_width;
  v.baseColorTexture.height = c->base_color_texture_height;
  v.baseColorTexture.components =
    c->base_color_texture_components ? c->base_color_texture_components : 3;
  v.baseColorTexture.emissive = c->base_color_texture_emissive != 0;
  return v;
}

//----------------------------------------------------------------------------
// Concrete mesh_view holding a zero-copy snapshot of the caller's arrays. The data
// pointers inside View reference caller-owned memory; only the small metadata (names,
// layout, vectors of scalar descriptors) is owned here.
class c_mesh_view : public f3d::mesh_view
{
public:
  c_mesh_view(std::string name, std::array<double, 2> range, f3d::mesh_view::memory_view_t view)
    : Name(std::move(name))
    , Range(range)
    , View(std::move(view))
  {
  }

  std::string getName() const override
  {
    return this->Name;
  }

  std::array<double, 2> getTimeRange() const override
  {
    return this->Range;
  }

  f3d::mesh_view::memory_view_t getMemoryView(double) const override
  {
    return this->View;
  }

private:
  std::string Name;
  std::array<double, 2> Range;
  f3d::mesh_view::memory_view_t View;
};
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
int f3d_scene_add_mesh_view(
  f3d_scene_t* scene, const f3d_memory_view_t* view, const char* name, double t_min, double t_max)
{
  if (!scene || !view)
  {
    return 0;
  }

  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);

  try
  {
    cpp_scene->add(std::make_shared<c_mesh_view>(name ? std::string(name) : std::string(),
      std::array<double, 2>{ t_min, t_max }, to_cpp_memory_view(view)));
  }
  catch (const f3d::scene::load_failure_exception& e)
  {
    f3d::log::error("Failed to add mesh view to scene: {}", e.what());
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
double* f3d_scene_get_animation_keyframes(f3d_scene_t* scene, unsigned int* count)
{
  f3d::scene* cpp_scene = reinterpret_cast<f3d::scene*>(scene);
  std::vector<double> keyframes = cpp_scene->getAnimationKeyFrames();
  *count = keyframes.size();
  double* times = new double[keyframes.size()];
  for (size_t i = 0; i < keyframes.size(); ++i)
  {
    times[i] = keyframes[i];
  }
  return times;
}

//----------------------------------------------------------------------------
void f3d_scene_free_animation_keyframes(double* keyframes)
{
  delete[] keyframes;
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
