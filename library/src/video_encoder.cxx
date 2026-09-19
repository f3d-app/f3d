#include "video_encoder.h"

#ifdef F3D_MODULE_FFMPEG
#include "video_encoder_ffmpeg.h"
#endif

namespace f3d
{
//----------------------------------------------------------------------------
std::shared_ptr<video_encoder> video_encoder::create(const params& p)
{
#ifdef F3D_MODULE_FFMPEG
  return std::make_shared<detail::video_encoder_ffmpeg>(p);
#else
  throw codec_exception("Video encoder is not supported, please build f3d with FFMPEG support");
#endif
}

//----------------------------------------------------------------------------
std::vector<std::pair<std::string, std::string>> video_encoder::getAvailableEncoders()
{
#ifdef F3D_MODULE_FFMPEG
  return detail::video_encoder_ffmpeg::getAvailableEncoders();
#else
  return {};
#endif
}

//----------------------------------------------------------------------------
video_encoder::codec_exception::codec_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
video_encoder::transport_exception::transport_exception(const std::string& what)
  : exception(what)
{
}

} // namespace f3d
