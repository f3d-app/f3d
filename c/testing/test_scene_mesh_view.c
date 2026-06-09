#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>

#include <stdio.h>
#include <string.h>

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
  memset(&view, 0, sizeof(view)); // zero => no normals/texcoords/texture, identity transform
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

  // Static mesh (t_min == t_max).
  if (f3d_scene_add_mesh_view(scene, &view, "triangle", 0.0, 0.0) != 1)
  {
    puts("[ERROR] f3d_scene_add_mesh_view failed");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_clear(scene);
  f3d_engine_delete(engine);
  return 0;
}
