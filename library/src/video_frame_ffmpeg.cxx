#include "video_frame_ffmpeg.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace f3d::detail
{
class F3D_EXPORT video_frame_ffmpeg::internals
{
public:
  internals() = default;
  ~internals()
  {
    av_frame_unref(&this->Frame);
  }

  internals(const internals&) = delete;
  internals& operator=(const internals&) = delete;

  AVFrame Frame{};
};

//----------------------------------------------------------------------------
video_frame_ffmpeg::video_frame_ffmpeg(int width, int height)
{
  if (width % 2 != 0 || height % 2 != 0)
  {
    throw invalid_frame_exception("Video frame size must be even");
  }

  this->Internals = std::make_unique<video_frame_ffmpeg::internals>();
  this->Internals->Frame.format = AV_PIX_FMT_YUV420P;
  this->Internals->Frame.width = width;
  this->Internals->Frame.height = height;
  this->Internals->Frame.linesize[0] = width;
  this->Internals->Frame.linesize[1] = width / 2;
  this->Internals->Frame.linesize[2] = width / 2;

  if (av_frame_get_buffer(&this->Internals->Frame, 32) < 0)
  {
    throw invalid_frame_exception("Failed to allocate video frame buffer");
  }
}

//----------------------------------------------------------------------------
video_frame_ffmpeg::~video_frame_ffmpeg() = default;

//----------------------------------------------------------------------------
std::byte* video_frame_ffmpeg::GetYPlane() const
{
  return reinterpret_cast<std::byte*>(this->Internals->Frame.data[0]);
}

//----------------------------------------------------------------------------
std::byte* video_frame_ffmpeg::GetUPlane() const
{
  return reinterpret_cast<std::byte*>(this->Internals->Frame.data[1]);
}

//----------------------------------------------------------------------------
std::byte* video_frame_ffmpeg::GetVPlane() const
{
  return reinterpret_cast<std::byte*>(this->Internals->Frame.data[2]);
}

//----------------------------------------------------------------------------
video_frame_ffmpeg& video_frame_ffmpeg::setTimestamp(int64_t timestamp)
{
  this->Internals->Frame.pts = timestamp;
  return *this;
}

//----------------------------------------------------------------------------
const void* video_frame_ffmpeg::GetHandle() const
{
  return &this->Internals->Frame;
}
} // namespace f3d
