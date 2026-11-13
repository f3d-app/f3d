#ifndef f3d_video_frame_ffmpeg_h
#define f3d_video_frame_ffmpeg_h

#include "video_frame.h"

#include <memory>

namespace f3d::detail
{
/**
 * @class   video_frame_ffmpeg
 * @brief   A concrete implementation of video_frame
 *
 * A concrete implementation of video_frame using FFmpeg.
 * Supports YUV420p pixel format only.
 */
class video_frame_ffmpeg : public video_frame
{
public:
  ///@{ @name Constructors
  /**
   * Default/copy/move constructors/operators.
   * The constructor can throw a video_frame_ffmpeg::invalid_frame_exception if the resolution
   * is not even or positive, or if F3D_MODULE_FFMPEG is not enabled.
   */
  video_frame_ffmpeg(int width, int height);
  ~video_frame_ffmpeg() override;
  video_frame_ffmpeg(const video_frame_ffmpeg& img) = delete;
  video_frame_ffmpeg& operator=(const video_frame_ffmpeg& img) = delete;
  video_frame_ffmpeg(video_frame_ffmpeg&& img) = delete;
  video_frame_ffmpeg& operator=(video_frame_ffmpeg&& img) = delete;
  ///@}

  /**
   * Documented public API
   * @see video_frame.h
   */
  video_frame_ffmpeg& setTimestamp(int64_t timestamp) override;

  /**
   * Implementation only API.
   * Get the Y plane pointer of the video frame.
   * Used by the window class to fill the Y plane of the video frame.
   */
  std::byte* GetYPlane() const;

  /**
   * Implementation only API.
   * Get the U plane pointer of the video frame.
   * Used by the window class to fill the U plane of the video frame.
   */
  std::byte* GetUPlane() const;

  /**
   * Implementation only API.
   * Get the V plane pointer of the video frame.
   * Used by the window class to fill the V plane of the video frame.
   */
  std::byte* GetVPlane() const;

  /**
   * Implementation only API.
   * Get the handle of the video frame.
   * The handle is an opaque pointer to the underlying video frame data structure.
   * It's used internally by the video_encoder class and should not be used directly.
   */
  const void* GetHandle() const;

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

#endif
