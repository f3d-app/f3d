#ifndef f3d_types_h
#define f3d_types_h

#include "export.h"

#include <array>
#include <cmath>
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
struct vector3_t : std::array<double, 3>
{
  vector3_t() = default;
  vector3_t(double x, double y, double z)
    : array{ x, y, z }
  {
  }

  static vector3_t fromSphericalCoordinates(double theta, double phi)
  {
    return { std::sin(phi) * std::cos(theta), std::sin(phi) * std::cos(theta), std::cos(phi) };
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
};
}

#endif
