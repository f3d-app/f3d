#ifndef f3d_types_h
#define f3d_types_h

#include "exception.h"
#include "export.h"

#include <algorithm>
#include <array>
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
  double Value;
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

private:
  std::array<double, N> Array{ 0 };
};

/**
 * Describe a RGB color.
 */
class color_t : public double_array_t<3>
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
    // XXX: No better way to do this ?
    return f3d::vector3_t{ this->x(), this->y(), this->z() };
  }
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
}

#endif
