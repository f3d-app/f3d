#include "video_packet_ffmpeg.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace f3d::detail
{
class video_packet_ffmpeg::internals
{
public:
  internals() = default;
  ~internals()
  {
    av_packet_unref(&this->Packet);
  }

  internals(const internals&) = delete;
  internals& operator=(const internals&) = delete;

  AVPacket Packet{};
};

//----------------------------------------------------------------------------
video_packet_ffmpeg::video_packet_ffmpeg()
  : Internals(std::make_unique<video_packet_ffmpeg::internals>())
{
}

//----------------------------------------------------------------------------
video_packet_ffmpeg::~video_packet_ffmpeg() = default;

//----------------------------------------------------------------------------
size_t video_packet_ffmpeg::getPacketSize() const
{
  return this->Internals->Packet.size;
}

//----------------------------------------------------------------------------
std::byte* video_packet_ffmpeg::getPacketData() const
{
  return reinterpret_cast<std::byte*>(this->Internals->Packet.data);
}

//----------------------------------------------------------------------------
int64_t video_packet_ffmpeg::getTimestamp() const
{
  return this->Internals->Packet.pts;
}

//----------------------------------------------------------------------------
bool video_packet_ffmpeg::isKeyFrame() const
{
  return this->Internals->Packet.flags & AV_PKT_FLAG_KEY;
}

//----------------------------------------------------------------------------
void* video_packet_ffmpeg::GetHandle()
{
  return &this->Internals->Packet;
}

} // namespace f3d
