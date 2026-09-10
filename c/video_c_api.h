#ifndef F3D_VIDEO_C_API_H
#define F3D_VIDEO_C_API_H

#include "export.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::video_frame object.
   */
  typedef struct f3d_video_frame_t f3d_video_frame_t;

  /**
   * @brief Opaque handle to an f3d::video_packet object.
   */
  typedef struct f3d_video_packet_t f3d_video_packet_t;

  /**
   * @brief Opaque handle to an f3d::video_encoder object.
   */
  typedef struct f3d_video_encoder_t f3d_video_encoder_t;

  /**
   * @brief Delete a video frame object.
   *
   * If provided frame is NULL, do nothing.
   * The frame handle is created from f3d_window_get_video_frame().
   *
   * @param frame Video frame handle to delete.
   */
  F3D_EXPORT void f3d_video_frame_delete(f3d_video_frame_t* frame);

  /**
   * @brief Set the timestamp of the video frame.
   *
   * @param frame Video frame handle.
   * @param timestamp Timestamp value.
   */
  F3D_EXPORT void f3d_video_frame_set_timestamp(f3d_video_frame_t* frame, int64_t timestamp);

  /**
   * @brief Get the size of the video packet in bytes.
   *
   * @param packet Video packet handle.
   * @return Packet size in bytes, or 0 if packet is NULL.
   */
  F3D_EXPORT size_t f3d_video_packet_get_packet_size(const f3d_video_packet_t* packet);

  /**
   * @brief Get the data of the video packet.
   *
   * @param packet Video packet handle.
   * @return Pointer to the packet data, or NULL if packet is NULL.
   */
  F3D_EXPORT const unsigned char* f3d_video_packet_get_packet_data(
    const f3d_video_packet_t* packet);

  /**
   * @brief Get the timestamp of the video packet.
   *
   * @param packet Video packet handle.
   * @return Timestamp value, or 0 if packet is NULL.
   */
  F3D_EXPORT int64_t f3d_video_packet_get_timestamp(const f3d_video_packet_t* packet);

  /**
   * @brief Get the packet type of the video packet.
   *
   * @param packet Video packet handle.
   * @return 1 if the packet is a key frame, 0 if not
   */
  F3D_EXPORT int f3d_video_packet_is_key_frame(const f3d_video_packet_t* packet);

  /**
   * @brief Enumeration of codec types.
   */
  typedef enum f3d_video_encoder_codec_t
  {
    F3D_VIDEO_ENCODER_CODEC_H264,
    F3D_VIDEO_ENCODER_CODEC_HEVC,
    F3D_VIDEO_ENCODER_CODEC_VP8,
    F3D_VIDEO_ENCODER_CODEC_VP9,
    F3D_VIDEO_ENCODER_CODEC_AV1,
    F3D_VIDEO_ENCODER_CODEC_EXPLICIT
  } f3d_video_encoder_codec_t;

  /**
   * @brief Parameters for configuring the video encoder construction.
   */
  typedef struct f3d_video_encoder_params_t
  {
    f3d_video_encoder_codec_t codec;
    const char* explicit_codec_name;
    int width;
    int height;
    double frame_rate;
    double bitrate;
    int low_latency;
  } f3d_video_encoder_params_t;

  /**
   * @brief Callback invoked by f3d_video_encoder_listen() for each encoded packet.
   *
   * This is called from a background thread owned by the encoder. The provided packet handle
   * is only valid for the duration of the call and must not be stored or deleted.
   *
   * @param packet Video packet handle, valid only during the callback.
   * @param user_data Arbitrary pointer passed to f3d_video_encoder_listen().
   */
  typedef void (*f3d_video_packet_callback_t)(const f3d_video_packet_t* packet, void* user_data);

  /**
   * @brief Get the list of available video encoders.
   *
   * @param names Array to be filled with encoder names. Must be deleted using
   * f3d_video_encoder_free_available_encoders.
   * @param descriptions Array to be filled with encoder descriptions. Must be deleted using
   * f3d_video_encoder_free_available_encoders.
   * @param count Pointer to an integer to receive the number of available encoders.
   */
  F3D_EXPORT void f3d_video_encoder_get_available_encoders(
    char*** names, char*** descriptions, int* count);

  /**
   * @brief Free the memory allocated by f3d_video_encoder_get_available_encoders.
   *
   * @param names Array of encoder names to free.
   * @param descriptions Array of encoder descriptions to free.
   * @param count Number of encoders in the arrays.
   */
  F3D_EXPORT void f3d_video_encoder_free_available_encoders(
    char** names, char** descriptions, int count);

  /**
   * @brief Create a new video encoder.
   *
   * Returns NULL if the codec could not be initialized, for example if F3D_MODULE_FFMPEG is
   * disabled or no valid codec is available.
   * The encoder handle must be deleted with f3d_video_encoder_delete().
   *
   * @param params Encoder parameters.
   * @return Video encoder handle, or NULL on failure.
   */
  F3D_EXPORT f3d_video_encoder_t* f3d_video_encoder_new(const f3d_video_encoder_params_t* params);

  /**
   * @brief Delete a video encoder object.
   *
   * If provided encoder is NULL, do nothing.
   *
   * @param encoder Video encoder handle to delete.
   */
  F3D_EXPORT void f3d_video_encoder_delete(f3d_video_encoder_t* encoder);

  /**
   * @brief Get the width of the video encoder.
   *
   * @param encoder Video encoder handle.
   * @return Width in pixels, or 0 if encoder is NULL.
   */
  F3D_EXPORT int f3d_video_encoder_get_width(const f3d_video_encoder_t* encoder);

  /**
   * @brief Get the height of the video encoder.
   *
   * @param encoder Video encoder handle.
   * @return Height in pixels, or 0 if encoder is NULL.
   */
  F3D_EXPORT int f3d_video_encoder_get_height(const f3d_video_encoder_t* encoder);

  /**
   * @brief Asynchronously listen for encoded video packets on a background thread.
   *
   * @param encoder Video encoder handle.
   * @param callback Function invoked for each encoded packet.
   * @param user_data Arbitrary pointer passed back to the callback.
   */
  F3D_EXPORT void f3d_video_encoder_listen(
    f3d_video_encoder_t* encoder, f3d_video_packet_callback_t callback, void* user_data);

  /**
   * @brief Send a frame to the video frame for encoding.
   *
   * @param encoder Video encoder handle.
   * @param frame Video frame handle.
   * @return 1 if the frame was submitted, 0 if the encoder is busy or -1 if an error occurred.
   */
  F3D_EXPORT int f3d_video_encoder_submit(f3d_video_encoder_t* encoder, f3d_video_frame_t* frame);

  /**
   * @brief Flush the video stream, ensuring all encoded frames are processed.
   *
   * @param encoder Video encoder handle.
   */
  F3D_EXPORT void f3d_video_encoder_flush(f3d_video_encoder_t* encoder);

#ifdef __cplusplus
}
#endif

#endif // F3D_VIDEO_C_API_H
