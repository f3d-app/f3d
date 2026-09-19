#ifndef f3d_video_packet_ffmpeg_h
#define f3d_video_packet_ffmpeg_h

#include "video_packet.h"

#include <memory>

namespace f3d::detail
{
/**
 * @class   video_packet_ffmpeg
 * @brief   A concrete implementation of video_packet
 *
 * A concrete implementation of video_packet using FFmpeg.
 */
class video_packet_ffmpeg : public video_packet
{
public:
  ///@{ @name Constructors
  /**
   * Default/copy/move constructors/operators.
   */
  video_packet_ffmpeg();
  ~video_packet_ffmpeg() override;
  video_packet_ffmpeg(const video_packet_ffmpeg& packet) = delete;
  video_packet_ffmpeg& operator=(const video_packet_ffmpeg& packet) = delete;
  video_packet_ffmpeg(video_packet_ffmpeg&& packet) noexcept = delete;
  video_packet_ffmpeg& operator=(video_packet_ffmpeg&& packet) noexcept = delete;
  ///@}

  ///@{
  /**
   * Documented public API
   * @see video_packet.h
   */
  size_t getPacketSize() const override;
  std::byte* getPacketData() const override;
  int64_t getTimestamp() const override;
  bool isKeyFrame() const override;
  ///@}

  /**
   * Implementation only API.
   * Get the handle of the video packet.
   * The handle is an opaque pointer to the underlying video packet data structure.
   * It's used internally by the video_encoder class and should not be used directly.
   */
  void* GetHandle();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
