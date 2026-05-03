#ifndef f3d_reader_types_h
#define f3d_reader_types_h

#include <cstdint>

namespace f3d
{
namespace reader_types
{

enum class file_availability : std::uint8_t
{
  AVAILABLE = 1,
  UNSUPPORTED_EXSTENSION = 2,
  UNSUPPORTED_CONTENT = 3,
};

} // namespace reader_types
} // namespace f3d

#endif
