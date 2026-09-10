#ifndef f3d_video_packet_h
#define f3d_video_packet_h

#include "exception.h"
#include "export.h"

namespace f3d
{
/**
 * @class   video_packet
 * @brief   Class used to represent a video packet
 *
 * A class to represent a video packet, which is an encoded video frame.
 * The video packet can be used to write to a file or stream over a network.
 * The video packet is produced by the video_encoder class.
 */
class F3D_EXPORT video_packet
{
public:
  virtual ~video_packet() = default;

  /**
   * Get the size of the video packet in bytes.
   */
  virtual size_t getPacketSize() const = 0;

  /**
   * Get the data of the video packet.
   */
  virtual std::byte* getPacketData() const = 0;

  /**
   * Get the timestamp of the video packet.
   */
  virtual int64_t getTimestamp() const = 0;

  /**
   * Returns true if the packet is a key frame, false otherwise.
   */
  virtual bool isKeyFrame() const = 0;
};
}

#endif
