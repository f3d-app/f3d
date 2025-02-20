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
  template<typename... Args>
  point3_t(Args&&... args)
    : array({ double(std::forward<Args>(args))... })
  {
  }
};

/**
 * Describe a 3D vector.
 */
struct F3D_EXPORT vector3_t : std::array<double, 3>
{
  template<typename... Args>
  vector3_t(Args&&... args)
    : array({ double(std::forward<Args>(args))... })
  {
  }
};

/**
 * Describe an angle in degrees.
 */
using angle_deg_t = double;

/**
 * Describe a ratio.
 */
class F3D_EXPORT ratio_t
{
public:
  ratio_t() = default;
  ratio_t(double val)
    : Value(val)
  {
  }
  operator double() const
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
class F3D_EXPORT double_array_t
{
public:
  double_array_t() = default;
  double_array_t(const std::vector<double>& vec)
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
  operator std::vector<double>() const
  {
    return std::vector<double>(this->Array.begin(), this->Array.end());
  }
  bool operator==(const double_array_t& other) const
  {
    return this->Array == other.Array;
  }
  bool operator!=(const double_array_t& other) const
  {
    return this->Array != other.Array;
  }
  double operator[](size_t i) const
  {
    if (i >= N)
    {
      throw f3d::type_access_exception("Incorrect index");
    }
    return this->Array[i];
  }
  double& operator[](size_t i)
  {
    if (i >= N)
    {
      throw f3d::type_access_exception("Incorrect index");
    }
    return this->Array[i];
  }
  const double* data() const
  {
    return this->Array.data();
  }

private:
  std::array<double, N> Array{ 0 };
};

/**
 * Describe a RGB color.
 */
class F3D_EXPORT color_t : public double_array_t<3>
{
public:
  color_t() = default;
  color_t(const std::vector<double>& vec)
    : double_array_t(vec)
  {
  }
  color_t(const std::initializer_list<double>& list)
    : double_array_t(list)
  {
  }
  color_t(double red, double green, double blue)
  {
    (*this)[0] = red;
    (*this)[1] = green;
    (*this)[2] = blue;
  }
  double r() const
  {
    return (*this)[0];
  }
  double g() const
  {
    return (*this)[1];
  }
  double b() const
  {
    return (*this)[2];
  }
};

/**
 * Describe a 3D direction.
 */
class F3D_EXPORT direction_t : public double_array_t<3>
{
public:
  direction_t() = default;
  direction_t(const std::vector<double>& vec)
    : double_array_t(vec)
  {
  }
  direction_t(const std::initializer_list<double>& list)
    : double_array_t(list)
  {
  }
  direction_t(double x, double y, double z)
  {
    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
  }
  double x() const
  {
    return (*this)[0];
  }
  double y() const
  {
    return (*this)[1];
  }
  double z() const
  {
    return (*this)[2];
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
