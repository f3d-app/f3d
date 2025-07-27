#include "types.h"

#include "vtkMath.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>

namespace f3d
{
//----------------------------------------------------------------------------
std::pair<bool, std::string> mesh_t::isValid() const
{
  if (this->points.size() == 0)
  {
    return { false, "The points buffer must not be empty." };
  }

  if (this->points.size() % 3 != 0)
  {
    std::string err = "The points buffer is not a multiple of 3. It's length is ";
    err += std::to_string(this->points.size());
    return { false, std::move(err) };
  }

  size_t nbPoints = this->points.size() / 3;

  if (this->normals.size() > 0 && this->normals.size() != nbPoints * 3)
  {
    return { false, "The normals buffer must be empty or equal to 3 times the number of points." };
  }

  if (this->texture_coordinates.size() > 0 && this->texture_coordinates.size() != nbPoints * 2)
  {
    return { false,
      "The texture_coordinates buffer must be empty or equal to 2 times the number of points." };
  }

  unsigned int expectedSize = std::accumulate(this->face_sides.begin(), this->face_sides.end(), 0);

  if (this->face_indices.size() != expectedSize)
  {
    std::string err = "The face_indices buffer size is invalid, it should be ";
    err += std::to_string(expectedSize);
    return { false, std::move(err) };
  }

  auto it = std::find_if(this->face_indices.cbegin(), this->face_indices.cend(),
    [=](unsigned int idx) { return idx >= nbPoints; });
  if (it != this->face_indices.cend())
  {
    std::string err = "Face vertex at index ";
    err += std::to_string(std::distance(this->face_indices.cbegin(), it));
    err += " is greater than the maximum vertex index (";
    err += std::to_string(nbPoints);
    err += ")";
    return { false, std::move(err) };
  }

  return { true, {} };
}

// see function explanation in types.h for explanation of logic
//----------------------------------------------------------------------------
F3D_EXPORT transform2d_t::transform2d_t(
  const double_array_t<2>& scale, const double_array_t<2>& translate, const angle_deg_t& angle)
{
  double angleRad = std::isnan(angle) ? 0.0 : vtkMath::RadiansFromDegrees(angle);

  double sinA = std::sin(angleRad);
  double cosA = std::cos(angleRad);

  (*this)[0] = cosA * scale[0];
  (*this)[1] = -sinA * scale[1];
  (*this)[2] = translate[0];
  (*this)[3] = sinA * scale[0];
  (*this)[4] = cosA * scale[1];
  (*this)[5] = translate[1];
  (*this)[6] = 0;
  (*this)[7] = 0;
  (*this)[8] = 1;

  // remove negative 0.0 wherever it occurs
  for (int i = 0; i < 9; i++)
  {
    if ((*this)[i] == 0.0 && std::signbit((*this)[i]))
    {
      (*this)[i] = 0.0;
    }
  }
}

}
