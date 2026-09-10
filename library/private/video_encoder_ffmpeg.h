#ifndef f3d_video_encoder_ffmpeg_h
#define f3d_video_encoder_ffmpeg_h

#include "video_encoder.h"

#include <memory>

namespace f3d::detail
{
/**
 * @class   video_encoder_ffmpeg
 * @brief   A concrete implementation of video_encoder
 *
 * A concrete implementation of video_encoder using FFmpeg.
 * Supports H264, HEVC, VP8, VP9, and AV1 codecs.
 */
class video_encoder_ffmpeg : public video_encoder
{
public:
  ///@{ @name Constructors
  /**
   * Default/copy/move constructors/operators.
   * The constructor throws a codec_exception if the codec cannot be initialized.
   */
  explicit video_encoder_ffmpeg(const params& p);
  ~video_encoder_ffmpeg() override;
  video_encoder_ffmpeg(const video_encoder_ffmpeg& stream) = delete;
  video_encoder_ffmpeg& operator=(const video_encoder_ffmpeg& stream) = delete;
  video_encoder_ffmpeg(video_encoder_ffmpeg&& stream) noexcept = delete;
  video_encoder_ffmpeg& operator=(video_encoder_ffmpeg&& stream) noexcept = delete;
  ///@}

  ///@{
  /**
   * Documented public API
   * @see video_encoder.h
   */
  static std::vector<std::pair<std::string, std::string>> getAvailableEncoders();
  int getWidth() const override;
  int getHeight() const override;
  video_encoder_ffmpeg& listen(
    std::function<void(const std::shared_ptr<video_packet>&)> callback) override;
  bool submit(const std::shared_ptr<video_frame>& frame) override;
  video_encoder_ffmpeg& flush() override;
  ///@}

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
