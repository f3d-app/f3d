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
 * visualize the user data without having to convert and copy to F3D internal structures. It also
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
        return f.template operator()<double>();
    }
  }

  /**
   * Structure representing a scalar array to be associated to the mesh points or faces.
   */
  struct data_array_t
  {
    std::string name;
    data_type type = data_type::F32;
    const void* data = nullptr;
    size_t components = 1;
    size_t stride = 1;
  };

  /**
   * Structure representing a view of the mesh in memory at a given time.
   * The pointers provided in this structure must remain valid once the mesh is added to the scene.
   * Will throw a load_failure_exception if any of this assumptions is not respected:
   * - points must have a 3 components and a data type of F32 or F64
   * - normals can be empty or must have a 3 components and a data type of F32 or F64
   * - texture_coordinates can be empty or must have a 2 components and a data type of F32 or F64
   * - faceOffsets can be empty or must have 1 component and a data type of U16, U32 or U64
   * - faceIndices can be empty or must have 1 component and a data type of U16, U32 or U64
   */
  struct memory_view_t
  {
    // points
    size_t pointCount = 0;
    data_array_t points;
    data_array_t normals;
    data_array_t textureCoordinates;

    // faces
    size_t faceOffsetCount = 0;
    data_array_t faceOffsets;
    size_t faceIndexCount = 0;
    data_array_t faceIndices;

    // scalars
    std::vector<data_array_t> pointScalars;
    std::vector<data_array_t> faceScalars;
  };

  /**
   * Specify the mesh data by providing a view of the mesh in memory at a given time.
   * Make sure to add a thread synchronization mechanism if the mesh data is updated asynchronously.
   */
  [[nodiscard]] virtual memory_view_t getMemoryView(double time) const = 0;

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
