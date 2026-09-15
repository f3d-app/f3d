#include <engine_c_api.h>

#include <stdio.h>
#include <string.h>

const f3d_memory_view_t* get_memory_view_callback(double time, void* opaque)
{
  return (const f3d_memory_view_t*)(opaque);
}

int test_scene_mesh_view()
{
  f3d_engine_t* engine = f3d_engine_create(1); // offscreen
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_scene_t* scene = f3d_engine_get_scene(engine);
  if (!scene)
  {
    puts("[ERROR] Failed to get scene");
    f3d_engine_delete(engine);
    return 1;
  }

  // A single triangle with a per-point scalar. All arrays are caller-owned and stay alive
  // for the whole function: f3d_scene_add_mesh_view keeps references, it does not copy them.
  float points[] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f };
  unsigned int offsets[] = { 0, 3 }; // one polygon of 3 corners (offset_count = cells + 1)
  unsigned int indices[] = { 0, 1, 2 };
  float height[] = { 0.0f, 0.5f, 1.0f };

  f3d_data_array_t point_scalar;
  memset(&point_scalar, 0, sizeof(point_scalar));
  point_scalar.name = "height";
  point_scalar.type = F3D_MESH_DATA_F32;
  point_scalar.data = height;
  point_scalar.components = 1;

  f3d_memory_view_t view;
  memset(&view, 0, sizeof(view));
  view.point_count = 3;
  view.points.type = F3D_MESH_DATA_F32;
  view.points.data = points;
  view.points.components = 3;
  view.polygons.offset_count = 2;
  view.polygons.offsets.type = F3D_MESH_DATA_U32;
  view.polygons.offsets.data = offsets;
  view.polygons.index_count = 3;
  view.polygons.indices.type = F3D_MESH_DATA_U32;
  view.polygons.indices.data = indices;
  view.point_scalars = &point_scalar;
  view.point_scalars_count = 1;

  f3d_mesh_view_t mesh_view;
  memset(&mesh_view, 0, sizeof(mesh_view));
  mesh_view.name = "triangle";
  mesh_view.time_min = 0.0; // static
  mesh_view.time_max = 0.0;
  mesh_view.get_memory_view = get_memory_view_callback;
  mesh_view.opaque = &view;

  if (f3d_scene_add_mesh_view(scene, &mesh_view) != 1)
  {
    puts("[ERROR] f3d_scene_add_mesh_view failed");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_info_t info;
  if (f3d_scene_get_scene_info(scene, &info) != 1)
  {
    puts("[ERROR] f3d_scene_get_scene_info failed");
    f3d_engine_delete(engine);
    return 1;
  }

  if (info.number_of_files != 1 || info.number_of_actors != 1 || info.number_of_points != 3 ||
    info.number_of_cells != 1)
  {
    puts("[ERROR] Scene info is incorrect");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_engine_delete(engine);
  return 0;
}
