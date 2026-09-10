#include "video_frame.h"

namespace f3d
{
//----------------------------------------------------------------------------
video_frame::invalid_frame_exception::invalid_frame_exception(const std::string& what)
  : exception(what)
{
}
} // namespace f3d
