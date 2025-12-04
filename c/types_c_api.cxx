#include "types_c_api.h"
#include "types.h"
#include <cmath>
#include <cstring>

// f3d_color_t functions
//----------------------------------------------------------------------------
double f3d_color_r(const f3d_color_t* color)
{
  if (!color)
  {
    return 0.0;
  }
  return color->data[0];
}

//----------------------------------------------------------------------------
double f3d_color_g(const f3d_color_t* color)
{
  if (!color)
  {
    return 0.0;
  }
  return color->data[1];
}

//----------------------------------------------------------------------------
double f3d_color_b(const f3d_color_t* color)
{
  if (!color)
  {
    return 0.0;
  }
  return color->data[2];
}

//----------------------------------------------------------------------------
void f3d_color_set(f3d_color_t* color, double r, double g, double b)
{
  if (!color)
  {
    return;
  }
  color->data[0] = r;
  color->data[1] = g;
  color->data[2] = b;
}

// f3d_direction_t functions
//----------------------------------------------------------------------------
double f3d_direction_x(const f3d_direction_t* dir)
{
  if (!dir)
  {
    return 0.0;
  }
  return dir->data[0];
}

//----------------------------------------------------------------------------
double f3d_direction_y(const f3d_direction_t* dir)
{
  if (!dir)
  {
    return 0.0;
  }
  return dir->data[1];
}

//----------------------------------------------------------------------------
double f3d_direction_z(const f3d_direction_t* dir)
{
  if (!dir)
  {
    return 0.0;
  }
  return dir->data[2];
}

//----------------------------------------------------------------------------
void f3d_direction_set(f3d_direction_t* dir, double x, double y, double z)
{
  if (!dir)
  {
    return;
  }
  dir->data[0] = x;
  dir->data[1] = y;
  dir->data[2] = z;
}

// f3d_transform2d_t functions
//----------------------------------------------------------------------------
void f3d_transform2d_create(f3d_transform2d_t* transform, double scale_x, double scale_y,
  double translate_x, double translate_y, f3d_angle_deg_t angle_deg)
{
  if (!transform)
  {
    return;
  }

  try
  {
    f3d::double_array_t<2> scale({ scale_x, scale_y });
    f3d::double_array_t<2> translate({ translate_x, translate_y });
    f3d::transform2d_t cpp_transform(scale, translate, angle_deg);

    for (int i = 0; i < 9; ++i)
    {
      transform->data[i] = cpp_transform[i];
    }
  }
  catch (...)
  {
    // Initialize to identity on error
    for (int i = 0; i < 9; ++i)
    {
      transform->data[i] = (i % 4 == 0) ? 1.0 : 0.0;
    }
  }
}

// f3d_colormap_t functions
//----------------------------------------------------------------------------
void f3d_colormap_free(f3d_colormap_t* colormap)
{
  if (!colormap)
  {
    return;
  }
  delete[] colormap->data;
  colormap->data = nullptr;
  colormap->count = 0;
}

// f3d_mesh_t functions
//----------------------------------------------------------------------------
int f3d_mesh_is_valid(const f3d_mesh_t* mesh, char** error_message)
{
  if (!mesh)
  {
    if (error_message)
    {
      const char* msg = "Mesh pointer is NULL";
      *error_message = new char[std::strlen(msg) + 1];
      std::strcpy(*error_message, msg);
    }
    return 0;
  }

  try
  {
    f3d::mesh_t cpp_mesh;

    if (mesh->points && mesh->points_count > 0)
    {
      cpp_mesh.points.assign(mesh->points, mesh->points + mesh->points_count);
    }

    if (mesh->normals && mesh->normals_count > 0)
    {
      cpp_mesh.normals.assign(mesh->normals, mesh->normals + mesh->normals_count);
    }

    if (mesh->texture_coordinates && mesh->texture_coordinates_count > 0)
    {
      cpp_mesh.texture_coordinates.assign(
        mesh->texture_coordinates, mesh->texture_coordinates + mesh->texture_coordinates_count);
    }

    if (mesh->face_sides && mesh->face_sides_count > 0)
    {
      cpp_mesh.face_sides.assign(mesh->face_sides, mesh->face_sides + mesh->face_sides_count);
    }

    if (mesh->face_indices && mesh->face_indices_count > 0)
    {
      cpp_mesh.face_indices.assign(
        mesh->face_indices, mesh->face_indices + mesh->face_indices_count);
    }

    // Check validity
    auto [valid, msg] = cpp_mesh.isValid();

    if (!valid && error_message)
    {
      *error_message = new char[msg.size() + 1];
      std::strcpy(*error_message, msg.c_str());
    }

    return valid ? 1 : 0;
  }
  catch (...)
  {
    if (error_message)
    {
      const char* msg = "Exception during mesh validation";
      *error_message = new char[std::strlen(msg) + 1];
      std::strcpy(*error_message, msg);
    }
    return 0;
  }
}

// f3d_light_state_t functions
//----------------------------------------------------------------------------
void f3d_light_state_free(f3d_light_state_t* light_state)
{
  delete light_state;
}

//----------------------------------------------------------------------------
int f3d_light_state_equal(const f3d_light_state_t* a, const f3d_light_state_t* b)
{
  if (!a || !b)
  {
    return 0;
  }

  if (a->type != b->type)
  {
    return 0;
  }

  for (int i = 0; i < 3; ++i)
  {
    if (a->position[i] != b->position[i])
    {
      return 0;
    }
    if (a->color.data[i] != b->color.data[i])
    {
      return 0;
    }
    if (a->direction[i] != b->direction[i])
    {
      return 0;
    }
  }

  if (a->positional_light != b->positional_light)
  {
    return 0;
  }

  if (a->intensity != b->intensity)
  {
    return 0;
  }

  if (a->switch_state != b->switch_state)
  {
    return 0;
  }

  return 1;
}
