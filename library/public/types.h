#ifndef f3d_types_h
#define f3d_types_h

#include "exception.h"
#include "export.h"
#include "interactor.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace f3d
{
/**
 * An exception that can be thrown by any type if
 * it fails in the constructor for some reason.
 */
struct type_construction_exception : public exception
{
  explicit type_construction_exception(const std::string& what = "")
    : exception(what) {};
};

/**
 * An exception that can be thrown by any type if
 * it fails when accessing data for some reason.
 */
struct type_access_exception : public exception
{
  explicit type_access_exception(const std::string& what = "")
    : exception(what) {};
};

/**
 * Describe a 3D point.
 */
struct F3D_EXPORT point3_t : std::array<double, 3>
{
};

/**
 * Describe a 3D vector.
 */
struct F3D_EXPORT vector3_t : std::array<double, 3>
{
};

/**
 * Describe an angle in degrees.
 */
using angle_deg_t = double;

/**
 * Describe a ratio.
 */
class ratio_t
{
public:
  inline ratio_t() = default;
  inline explicit ratio_t(double val)
    : Value(val)
  {
  }
  inline operator double() const
  {
    return this->Value;
  }

private:
  double Value = 0;
};

/**
 * A base template type for an array of double
 */
template<unsigned int N>
class double_array_t
{
public:
  double_array_t() = default;
  explicit double_array_t(const std::vector<double>& vec)
  {
    if (vec.size() != N)
    {
      throw f3d::type_construction_exception("Provided vector does not have the right size");
    }
    std::copy_n(vec.begin(), N, this->Array.begin());
  }
  double_array_t(const std::initializer_list<double>& list)
  {
    if (list.size() != N)
    {
      throw f3d::type_construction_exception("Provided list does not have the right size");
    }
    std::copy_n(list.begin(), N, this->Array.begin());
  }
  [[nodiscard]] operator std::vector<double>() const
  {
    return std::vector<double>(this->Array.begin(), this->Array.end());
  }
  [[nodiscard]] bool operator==(const double_array_t& other) const
  {
    return this->Array == other.Array;
  }
  [[nodiscard]] bool operator!=(const double_array_t& other) const
  {
    return this->Array != other.Array;
  }
  [[nodiscard]] double operator[](size_t i) const
  {
    if (i >= N)
    {
      throw f3d::type_access_exception("Incorrect index");
    }
    return this->Array[i];
  }
  [[nodiscard]] double& operator[](size_t i)
  {
    if (i >= N)
    {
      throw f3d::type_access_exception("Incorrect index");
    }
    return this->Array[i];
  }
  [[nodiscard]] const double* data() const
  {
    return this->Array.data();
  }

protected:
  std::array<double, N> Array{ 0 };
};

/**
 * Describe a RGB color.
 */
class F3D_EXPORT color_t : public double_array_t<3>
{
public:
  inline color_t() = default;
  inline explicit color_t(const std::vector<double>& vec)
    : double_array_t(vec)
  {
  }
  inline color_t(const std::initializer_list<double>& list)
    : double_array_t(list)
  {
  }
  inline color_t(double red, double green, double blue)
  {
    (*this)[0] = red;
    (*this)[1] = green;
    (*this)[2] = blue;
  }
  [[nodiscard]] inline double r() const
  {
    return (*this)[0];
  }
  [[nodiscard]] inline double g() const
  {
    return (*this)[1];
  }
  [[nodiscard]] inline double b() const
  {
    return (*this)[2];
  }
};

/**
 * Describe a 3D direction.
 */
class direction_t : public double_array_t<3>
{
public:
  inline direction_t() = default;
  inline explicit direction_t(const std::vector<double>& vec)
    : double_array_t(vec)
  {
  }
  inline direction_t(const std::initializer_list<double>& list)
    : double_array_t(list)
  {
  }
  inline direction_t(double x, double y, double z)
  {
    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
  }
  [[nodiscard]] inline double x() const
  {
    return (*this)[0];
  }
  [[nodiscard]] inline double y() const
  {
    return (*this)[1];
  }
  [[nodiscard]] inline double z() const
  {
    return (*this)[2];
  }
  [[nodiscard]] operator f3d::vector3_t() const
  {
    return f3d::vector3_t{ this->Array };
  }
};

/**
 *  Store a 3x3 transform matrix as a sequence of 9 float values
 *  VTK expects a 4x4 matrix, but for 2d transforms this is exposed as a 3x3 matrix
 */

class transform2d_t : public double_array_t<9>
{
public:
  inline transform2d_t() = default;
  inline explicit transform2d_t(const std::vector<double>& vec)
    : double_array_t(vec)
  {
  }
  inline transform2d_t(const std::initializer_list<double>& list)
    : double_array_t(list)
  {
  }

  // clang-format off
  /**
   *  The variables of this function are based on the mathematical notation for matrices,
   *  where the coordinates correspond to the following:
   *
   *        [M1_1, M1_2, M1_3]
   *  M =   [M2_1, M2_2, M2_3]
   *        [M3_1, M3_2, M3_3]
   */
  // clang-format on
  inline transform2d_t(double M1_1, double M1_2, double M1_3, double M2_1, double M2_2, double M2_3,
    double M3_1, double M3_2, double M3_3)
  {
    (*this)[0] = M1_1;
    (*this)[1] = M1_2;
    (*this)[2] = M1_3;
    (*this)[3] = M2_1;
    (*this)[4] = M2_2;
    (*this)[5] = M2_3;
    (*this)[6] = M3_1;
    (*this)[7] = M3_2;
    (*this)[8] = M3_3;
  }

  // clang-format off
  /**
   *  The general form of a 3x3 transformation matrix M with scale S(x,y),
   *  translation T(x,y), and angle a (in degrees), is solved out to the following:
   *
   *      [cos(a)*S(x), -sin(a)*S(y),   T(x)]
   *  M = [sin(a)*S(x), cos(a)*S(y),    T(y)]
   *      [0,           0,              1   ]
   *
   *  Using this formula, we fill each cell using the values in the constructor
   */
  // clang-format on

  F3D_EXPORT transform2d_t(const double_array_t<2>& scale, const double_array_t<2>& translate,
    const angle_deg_t& angleRad);
};

/**
 * Describe a colormap, which is a vector of repeated:
 * val,r,g,b
 */
class colormap_t
{
public:
  colormap_t() = default;
  explicit colormap_t(const std::vector<double>& vec)
    : Vector(vec)
  {
  }
  colormap_t(const std::initializer_list<double>& list)
    : Vector(list)
  {
  }
  [[nodiscard]] operator std::vector<double>() const
  {
    return this->Vector;
  }
  [[nodiscard]] bool operator==(const colormap_t& other) const
  {
    return this->Vector == other.Vector;
  }
  [[nodiscard]] bool operator!=(const colormap_t& other) const
  {
    return this->Vector != other.Vector;
  }
  [[nodiscard]] const double* data() const
  {
    return this->Vector.data();
  }

protected:
  std::vector<double> Vector;
};

class bind_vector_t
{
public:
  bind_vector_t() = default;

  explicit bind_vector_t(const std::vector<interaction_bind_t>& vec)
    : Vector(vec)
  {
  }

  bind_vector_t(const std::initializer_list<interaction_bind_t>& list)
    : Vector(list)
  {
  }

  [[nodiscard]] operator std::vector<interaction_bind_t>() const
  {
    return this->Vector;
  }

  [[nodiscard]] bool operator==(const bind_vector_t& other) const
  {
    return this->Vector == other.Vector;
  }

  [[nodiscard]] bool operator!=(const bind_vector_t& other) const
  {
    return this->Vector != other.Vector;
  }

  [[nodiscard]] const interaction_bind_t* data() const
  {
    return this->Vector.data();
  }

protected:
  std::vector<interaction_bind_t> Vector;
};


/**
 * Describe a 3D surfacic mesh.
 * A valid mesh fulfills these requirements:
 * - points must not be empty and its length must be a multiple of 3 (3 times the number of points)
 * - normals can be empty or its length must be 3 times the number of points
 * - texture_coordinates can be empty or its length must be 2 times the number of points
 * - face_sides can be any size including empty resulting in a point cloud
 * - face_indices length must be the sum of all values in face_sides
 */
struct mesh_t
{
  std::vector<float> points;
  std::vector<float> normals;
  std::vector<float> texture_coordinates;
  std::vector<unsigned int> face_sides;
  std::vector<unsigned int> face_indices;

  /**
   * Check validity of the mesh.
   * Returns a pair with the first element to true if the mesh is valid.
   * If invalid, an error message is returned in the second element.
   */
  F3D_EXPORT std::pair<bool, std::string> isValid() const;
};

enum class F3D_EXPORT light_type : std::uint8_t
{
  HEADLIGHT = 1,
  CAMERA_LIGHT = 2,
  SCENE_LIGHT = 3,
};

struct F3D_EXPORT light_state_t
{
  light_type type = light_type::SCENE_LIGHT;
  point3_t position = { 0., 0., 0. };
  color_t color = { 1., 1., 1. };
  vector3_t direction = { 1., 0., 0. };
  bool positionalLight = false;
  double intensity = 1.0;
  bool switchState = true;

  [[nodiscard]] bool operator==(const light_state_t& other) const
  {
    return this->type == other.type && this->position == other.position &&
      this->color == other.color && this->direction == other.direction &&
      this->positionalLight == other.positionalLight && this->intensity == other.intensity &&
      this->switchState == other.switchState;
  }
};

}

#endif
