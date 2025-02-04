#ifndef f3d_types_h
#define f3d_types_h

#include "export.h"

#include <array>
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
 * Describe a RGB color.
 */
class F3D_EXPORT color_t
{
public:
  color_t() = default;
  color_t(std::vector<double> rgb)
    : RGB(rgb)
  {
  }
  color_t(double red, double green, double blue)
  {
    RGB.emplace_back(red);
    RGB.emplace_back(green);
    RGB.emplace_back(blue);
  }
  color_t(std::initializer_list<double> list)
  {
    this->RGB = std::vector<double>(list);
  }
  operator std::vector<double>() const
  {
    return this->RGB;
  }
  bool operator==(const color_t& other) const
  {
    return this->RGB == other.RGB;
  }
  bool operator!=(const color_t& other) const
  {
    return this->RGB != other.RGB;
  }
  const double* data() const
  {
    return this->RGB.data();
  }

private:
  std::vector<double> RGB;
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
