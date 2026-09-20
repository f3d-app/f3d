#ifndef f3d_video_frame_h
#define f3d_video_frame_h

#include "exception.h"
#include "export.h"

namespace f3d
{
/**
 * @class   video_frame
 * @brief   Class used to represent a video frame
 *
 * A class to represent a video frame, which is a raw video frame.
 * The video frame can be submitted to the video_encoder class.
 */
class F3D_EXPORT video_frame
{
public:
  virtual ~video_frame() = default;

  /**
   * Set the timestamp of the video frame.
   */
  virtual video_frame& setTimestamp(int64_t timestamp) = 0;

  /**
   * An exception that can be thrown
   * when video frame allocation fails.
   */
  struct invalid_frame_exception : public exception
  {
    explicit invalid_frame_exception(const std::string& what = "");
  };
};
}

#endif
