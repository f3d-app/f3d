/**
 * @namespace   types
 * @brief   Namespace containing types used in the libf3d API
 */

#ifndef f3d_types_h
#define f3d_types_h

#include <array>

namespace f3d
{
/**
 * Describe a 3D point
 */
struct point3_t : std::array<double, 3>
{
};

/**
 * Describe a 3D vector
 */
struct vector3_t : std::array<double, 3>
{
};

/**
 * Describe a 4x4 matrix
 */
struct matrix4_t : std::array<double, 16>
{
};

/**
 * Describe an angle in degrees
 */
using angleDeg_t = double;
}

#endif
