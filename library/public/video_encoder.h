#ifndef f3d_video_encoder_h
#define f3d_video_encoder_h

#include "exception.h"
#include "export.h"
#include "video_frame.h"
#include "video_packet.h"

/// @cond
#include <functional>
#include <memory>
/// @endcond

namespace f3d
{
/**
 * @class   video_encoder
 * @brief   Class used to represent a video stream
 *
 * A class to represent a video stream, used to encode frames
 * to video packets.
 * It supports a variety of codecs (H264, HEVC, VP8, VP9, AV1) as long as the encoder supports
 * yuv420p pixel format.
 */
class F3D_EXPORT video_encoder
{
public:
  virtual ~video_encoder() = default;

  /**
   * Enumeration of codec types.
   * Supported codecs include H264, HEVC, VP8, VP9, AV1, and EXPLICIT for specifying a codec name.
   * H264 is widely supported and commonly used for video encoding.
   * HEVC (also called H265) offers better compression efficiency than H264 but may have less
   * widespread support.
   * VP8 and VP9 are commonly used for web-based video applications.
   * AV1 is a modern codec with high compression efficiency but may require
   * more computational resources.
   */
  enum class codec
  {
    H264,
    HEVC,
    VP8,
    VP9,
    AV1,
    EXPLICIT
  };

  /**
   * Parameters for configuring the video stream construction.
   * ExplicitCodecName is used when the codec is set to EXPLICIT.
   * Use getAvailableEncoders() to retrieve the list of available codec names.
   * Bitrate is specified in Mbps.
   */
  struct params
  {
    codec Codec = codec::H264;
    std::string ExplicitCodecName;
    int Width = 0;
    int Height = 0;
    double FrameRate = 30;
    double Bitrate = 5.0;
    bool LowLatency = false;
  };

  /**
   * Create a video encoder instance.
   * Throws codec_exception if the codec cannot be initialized.
   */
  [[nodiscard]] static std::shared_ptr<video_encoder> create(const params& p);

  /**
   * Get the list of available codecs.
   */
  [[nodiscard]] static std::vector<std::pair<std::string, std::string>> getAvailableEncoders();

  ///@{
  /**
   * Get the width and height of the video stream.
   */
  [[nodiscard]] virtual int getWidth() const = 0;
  [[nodiscard]] virtual int getHeight() const = 0;
  ///@}

  /**
   * Asynchronously listen for encoded video packets and invoke the provided callback for each
   * packet. Thread safety is ensured, allowing concurrent calls to submit and listen.
   * Throws transport_exception if there is an error receiving packets from the encoder.
   */
  virtual video_encoder& listen(
    std::function<void(const std::shared_ptr<video_packet>&)> callback) = 0;

  /**
   * Send a frame to the video stream for encoding.
   * Returns true if the frame was successfully submitted, false if the encoder is busy and cannot
   * accept new frames at the moment. Thread safety is ensured, allowing concurrent calls to submit
   * and listen. Throws transport_exception if there is an error submitting the frame to the
   * encoder.
   */
  virtual bool submit(const std::shared_ptr<video_frame>& frame) = 0;

  /**
   * Flush the video stream, ensuring all encoded frames are processed.
   * Signals the end of the stream to the encoder, allowing it to output any remaining packets.
   */
  virtual video_encoder& flush() = 0;

  /**
   * An exception that can be thrown by the engine
   * when codec initialization fails.
   */
  struct codec_exception : public exception
  {
    explicit codec_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the engine
   * when data transport fails.
   */
  struct transport_exception : public exception
  {
    explicit transport_exception(const std::string& what = "");
  };
};
}

#endif
