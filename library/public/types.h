#ifndef f3d_types_h
#define f3d_types_h

#include "export.h"

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace f3d
{
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
      throw std::runtime_error("cannot create a vector3_t");
    }
    Value[0] = vec[0];
    Value[1] = vec[1];
    Value[2] = vec[2];
  }

  double* data()
  {
    return Value;
  }
  const double* data() const
  {
    return Value;
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

  double* begin()
  {
    return Value;
  }
  const double* begin() const
  {
    return Value;
  }
  const double* cbegin() const
  {
    return Value;
  }
  double* end()
  {
    return Value + 3;
  }
  const double* end() const
  {
    return Value + 3;
  }
  const double* cend() const
  {
    return Value + 3;
  }

  static vector3_t fromSphericalCoordinates(double theta, double phi)
  {
    auto sinPhi = std::sin(phi);
    auto cosTheta = std::cos(theta);
    return { sinPhi * cosTheta, sinPhi * cosTheta, std::cos(phi) };
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
  double Value[3];
};
}

#endif
