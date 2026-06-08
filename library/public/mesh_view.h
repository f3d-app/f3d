#ifndef f3d_mesh_view_h
#define f3d_mesh_view_h

#include "exception.h"
#include "export.h"

/// @cond
#include <array>
#include <cstdint>
#include <string>
#include <vector>
/// @endcond

namespace f3d
{
/**
 * @class   mesh_view
 * @brief   Abstract class to represent a 3D surfacic mesh view in memory
 *
 * The mesh_view class represents a 3D surfacic mesh with points, faces, normals and texture
 * coordinates. It is used to add meshes directly to the scene without having to write them to a
 * file. The interface allows to provide a view of the mesh in memory, making it possible to
 * visualize data without having to convert and copy to F3D internal structures. It also
 * allows to provide an animated mesh by providing different views of the mesh at different times.
 */
class F3D_EXPORT mesh_view
{
public:
  /**
   * Get the temporal range
   */
  [[nodiscard]] virtual std::array<double, 2> getTimeRange() const
  {
    return { 0.0, 0.0 };
  }

  /**
   * Get the name of the mesh
   */
  [[nodiscard]] virtual std::string getName() const
  {
    return "";
  }

  /**
   * Enumeration of supported scalar types for point and face scalars
   */
  enum class data_type : uint8_t
  {
    U8,
    I8,
    U16,
    I16,
    U32,
    I32,
    U64,
    I64,
    F32,
    F64
  };

  /**
   * Dispatch a functor based on the data type, used to avoid code duplication when handling
   * data of different types.
   */
  template<typename Functor>
  static decltype(auto) dataTypeDispatch(f3d::mesh_view::data_type t, Functor&& f)
  {
    switch (t)
    {
      case f3d::mesh_view::data_type::U8:
        return f.template operator()<uint8_t>();
      case f3d::mesh_view::data_type::I8:
        return f.template operator()<int8_t>();
      case f3d::mesh_view::data_type::U16:
        return f.template operator()<uint16_t>();
      case f3d::mesh_view::data_type::I16:
        return f.template operator()<int16_t>();
      case f3d::mesh_view::data_type::U32:
        return f.template operator()<uint32_t>();
      case f3d::mesh_view::data_type::I32:
        return f.template operator()<int32_t>();
      case f3d::mesh_view::data_type::U64:
        return f.template operator()<uint64_t>();
      case f3d::mesh_view::data_type::I64:
        return f.template operator()<int64_t>();
      case f3d::mesh_view::data_type::F32:
        return f.template operator()<float>();
      case f3d::mesh_view::data_type::F64:
      default:
        return f.template operator()<double>();
    }
  }

  /**
   * Structure representing a view of an existing data array.
   * `name` is optional but recommended for pointScalars and faceScalars.
   * `data` pointer must remain valid while the mesh is used in the scene.
   * `stride` is the number of elements (not bytes) to skip to get to the next tuple.
   * If `timeDependent` is true, it means that the data in the array can change over time.
   * Set it to false if the data in the array is constant over time, it can help improving
   * performance.
   */
  struct data_array_t
  {
    std::string name;
    data_type type = data_type::F32;
    const void* data = nullptr;
    size_t components = 1;
    size_t stride = 1;
    bool timeDependent = true;
  };

  /**
   * Structure representing a cell array.
   * For vertices and lines, `offsets` encodes the size of the polyvertex and polylines group
   * respectively. For polygons, `offsets` encodes the size of each polygons (number of vertices per
   * face). `offsetCount` must be equal to the number of cells + 1, and the last value in `offsets`
   * must be equal to `indexCount`. If `offsetCount` is 1, it means that there is no cell. Will
   * throw a load_failure_exception if any of this assumptions is not respected:
   * - offsetCount is less than 1
   * - offsets can be empty or must have 1 component and a data type of I32, U32, I64, or U64
   * - indices can be empty or must have 1 component and a data type of I32, U32, I64, or U64
   * - offsets and indices must have the same data type
   */
  struct cell_array_t
  {
    size_t offsetCount = 1;
    data_array_t offsets;
    size_t indexCount = 0;
    data_array_t indices;
  };

  /**
   * Structure representing an in-memory base-color (albedo) texture, sampled through the
   * mesh `textureCoordinates`. Avoids writing a temporary image file to disk. `data` is a
   * row-major uint8 buffer of `width * height * components` bytes, with `components` equal
   * to 3 (RGB) or 4 (RGBA). Leave `data` null (the default) for no texture. The pointer
   * must remain valid until the mesh is removed from the scene. When `emissive` is true the
   * same image is additionally installed as the emissive texture (for unlit/flat display).
   */
  struct texture_t
  {
    size_t width = 0;
    size_t height = 0;
    size_t components = 3;
    const void* data = nullptr;
    bool emissive = false;
  };

  /**
   * Structure representing a view of the mesh in memory at a given time.
   * The pointers provided in this structure must remain valid once the mesh is added to the scene.
   * Will throw a load_failure_exception if any of this assumptions is not respected:
   * - points must have a 3 components and a data type of F32 or F64
   * - normals can be empty or must have a 3 components and a data type of F32 or F64
   * - texture_coordinates can be empty or must have a 2 components and a data type of F32 or F64
   */
  struct memory_view_t
  {
    // points
    size_t pointCount = 0;
    data_array_t points;
    data_array_t normals;
    data_array_t textureCoordinates;

    // cells
    cell_array_t vertices;
    cell_array_t lines;
    cell_array_t polygons;

    // scalars
    std::vector<data_array_t> pointScalars;
    std::vector<data_array_t> cellScalars;

    // optional in-memory base-color texture (sampled via textureCoordinates)
    texture_t baseColorTexture;
  };

  /**
   * Specify the mesh data by providing a view of the mesh in memory at a given time.
   * Make sure to add a thread synchronization mechanism if the mesh data is updated asynchronously.
   */
  [[nodiscard]] virtual memory_view_t getMemoryView(double time) const = 0;

  /**
   * Structure representing a 3D affine transform applied to the mesh as a whole, as a single
   * 4x4 homogeneous `matrix` (row-major, i.e. `matrix[row * 4 + col]`). It can encode any
   * combination of translation, rotation and (anisotropic) scaling. The transform is applied
   * on the GPU at render time, so the mesh coordinates returned by getMemoryView() and the
   * axis labels are left unchanged. The default is the identity matrix (no transform). A
   * typical use is vertical exaggeration of a surface, encoded as a scale on one axis.
   */
  struct transform_3d
  {
    std::array<double, 16> matrix = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0 };
  };

  /**
   * Specify a 3D affine transform applied to the whole mesh at a given time. The default
   * implementation returns the identity transform (no transform). Override to translate,
   * rotate or scale the mesh, optionally as a function of time for animation.
   */
  [[nodiscard]] virtual transform_3d getTransform(double) const
  {
    return {};
  }

  //! @cond
  mesh_view() = default;
  virtual ~mesh_view() = default;
  mesh_view(const mesh_view&) = delete;
  mesh_view(mesh_view&&) = delete;
  mesh_view& operator=(const mesh_view&) = delete;
  mesh_view& operator=(mesh_view&&) = delete;
  //! @endcond
};
}

#endif
