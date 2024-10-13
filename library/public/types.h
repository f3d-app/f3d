#ifndef f3d_types_h
#define f3d_types_h

#include "exception.h"
#include "export.h"

#include <array>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

namespace f3d
{

/**
 * An exception that can be thrown when we fail to create a type
 */
struct type_creation_exception : public exception
{
  explicit type_creation_exception(const std::string& what = "")
    : exception(what)
  {
  }
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

/**
 * Describe a 3D vector.
 */
struct F3D_EXPORT vector3_t
{
  vector3_t() = default;
  vector3_t(double x, double y, double z)
    : Value{ x, y, z }
  {
  }
  vector3_t(const std::vector<double>& vec)
  {
    if (vec.size() != 3)
    {
      throw type_creation_exception("cannot create a vector3_t");
    }
    Value[0] = vec[0];
    Value[1] = vec[1];
    Value[2] = vec[2];
  }
  vector3_t(const std::array<double, 3>& arr)
  {
    Value[0] = arr[0];
    Value[1] = arr[1];
    Value[2] = arr[2];
  }
  vector3_t(const double* ptr)
  {
    Value[0] = ptr[0];
    Value[1] = ptr[1];
    Value[2] = ptr[2];
  }
  vector3_t(std::initializer_list<double> l)
  {
    if (l.size() != 3)
    {
      throw type_creation_exception("cannot create a vector3_t");
    }
    std::copy(l.begin(), l.end(), std::begin(Value));
  }

  double* data()
  {
    return Value.data();
  }
  const double* data() const
  {
    return Value.data();
  }

  double& operator[](int idx)
  {
    return Value[idx];
  }
  double operator[](int idx) const
  {
    return Value[idx];
  }
  operator std::vector<double>() const
  {
    return { Value[0], Value[1], Value[2] };
  }
  operator std::array<double, 3>() const
  {
    return { Value[0], Value[1], Value[2] };
  }
  bool operator==(const vector3_t& vec) const
  {
    return Value[0] == vec.Value[0] && Value[1] == vec.Value[1] && Value[2] == vec.Value[2];
  }
  bool operator!=(const vector3_t& vec) const
  {
    return !(*this == vec);
  }

  auto begin()
  {
    return Value.begin();
  }
  // auto begin() const
  // {
  //   return Value.begin();
  // }
  auto cbegin() const
  {
    return Value.cbegin();
  }
  auto end()
  {
    return Value.end();
  }
  // auto end() const
  // {
  //   return Value.end();
  // }
  auto cend() const
  {
    return Value.cend();
  }

  static vector3_t fromSphericalCoordinates(double theta, double phi)
  {
    auto sinPhi = std::sin(phi);
    return { sinPhi * std::cos(theta), sinPhi * std::sin(theta), std::cos(phi) };
  }
  static vector3_t x()
  {
    return { 1, 0, 0 };
  }
  static vector3_t y()
  {
    return { 0, 1, 0 };
  }
  static vector3_t z()
  {
    return { 0, 0, 1 };
  }
  static vector3_t zero()
  {
    return { 0, 0, 0 };
  }

private:
  std::array<double, 3> Value;
};

inline std::ostream& operator<<(std::ostream& os, const f3d::vector3_t& vec)
{
  for (int i = 0; i < 3; ++i)
  {
    os << (i == 0 ? ", " : "{ ") << vec[i];
  }
  os << " }";
  return os;
}

}

#endif
