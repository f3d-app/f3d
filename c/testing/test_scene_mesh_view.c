#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>
#include <window_c_api.h>

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

  // A textured quad: exercises the optional in-memory base-color texture path
  // (raw bytes -> f3d::image via setContent -> vtkTexture on the imported actor).
  float qpoints[] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
  float qtexcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
  unsigned int qoffsets[] = { 0, 4 }; // one quad polygon
  unsigned int qindices[] = { 0, 1, 2, 3 };
  // 2x2 RGB checker (row-major): red, green / blue, yellow.
  unsigned char texels[] = { 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 0 };

  f3d_memory_view_t qview;
  memset(&qview, 0, sizeof(qview));
  qview.point_count = 4;
  qview.points.type = F3D_MESH_DATA_F32;
  qview.points.data = qpoints;
  qview.points.components = 3;
  qview.texture_coordinates.type = F3D_MESH_DATA_F32;
  qview.texture_coordinates.data = qtexcoords;
  qview.texture_coordinates.components = 2;
  qview.polygons.offset_count = 2;
  qview.polygons.offsets.type = F3D_MESH_DATA_U32;
  qview.polygons.offsets.data = qoffsets;
  qview.polygons.index_count = 4;
  qview.polygons.indices.type = F3D_MESH_DATA_U32;
  qview.polygons.indices.data = qindices;
  qview.base_color_texture = texels;
  qview.base_color_texture_width = 2;
  qview.base_color_texture_height = 2;
  qview.base_color_texture_components = 3;
  qview.base_color_texture_emissive = 1; // also use as emissive (flat full-strength)

  if (f3d_scene_add_mesh_view(scene, &qview, "textured_quad", 0.0, 0.0) != 1)
  {
    puts("[ERROR] f3d_scene_add_mesh_view (textured) failed");
    f3d_engine_delete(engine);
    return 1;
  }

  // Render once so the base-color texture is actually built and applied.
  f3d_window_t* window = f3d_engine_get_window(engine);
  if (!window || f3d_window_render(window) != 1)
  {
    puts("[ERROR] render of textured mesh_view failed");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_clear(scene);
  f3d_engine_delete(engine);
  return 0;
}
