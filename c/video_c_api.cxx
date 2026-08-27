#include "video_c_api.h"
#include "log.h"
#include "video_encoder.h"
#include "window.h"
#include "window_c_api.h"

#include <cstring>

template<typename T>
struct SmartPointerWrapper
{
  std::shared_ptr<T> impl;

  SmartPointerWrapper(std::shared_ptr<T> ptr)
    : impl(std::move(ptr))
  {
  }
};

struct f3d_video_encoder_t : public SmartPointerWrapper<f3d::video_encoder>
{
};
struct f3d_video_frame_t : public SmartPointerWrapper<f3d::video_frame>
{
};

//----------------------------------------------------------------------------
void f3d_video_frame_delete(f3d_video_frame_t* frame)
{
  if (!frame)
  {
    f3d::log::warn("Video frame is null, cannot delete video frame");
    return;
  }

  delete reinterpret_cast<f3d_video_frame_t*>(frame);
}

//----------------------------------------------------------------------------
void f3d_video_frame_set_timestamp(f3d_video_frame_t* frame, int64_t timestamp)
{
  if (!frame)
  {
    return;
  }

  f3d_video_frame_t* cpp_frame = reinterpret_cast<f3d_video_frame_t*>(frame);
  cpp_frame->impl->setTimestamp(timestamp);
}

//----------------------------------------------------------------------------
size_t f3d_video_packet_get_packet_size(const f3d_video_packet_t* packet)
{
  if (!packet)
  {
    return 0;
  }

  const f3d::video_packet* cpp_packet = reinterpret_cast<const f3d::video_packet*>(packet);
  return cpp_packet->getPacketSize();
}

//----------------------------------------------------------------------------
const unsigned char* f3d_video_packet_get_packet_data(const f3d_video_packet_t* packet)
{
  if (!packet)
  {
    return nullptr;
  }

  const f3d::video_packet* cpp_packet = reinterpret_cast<const f3d::video_packet*>(packet);
  return reinterpret_cast<const unsigned char*>(cpp_packet->getPacketData());
}

//----------------------------------------------------------------------------
int64_t f3d_video_packet_get_timestamp(const f3d_video_packet_t* packet)
{
  if (!packet)
  {
    return 0;
  }

  const f3d::video_packet* cpp_packet = reinterpret_cast<const f3d::video_packet*>(packet);
  return cpp_packet->getTimestamp();
}

//----------------------------------------------------------------------------
F3D_EXPORT int f3d_video_packet_is_key_frame(const f3d_video_packet_t* packet)
{
  if (!packet)
  {
    return 0;
  }

  const f3d::video_packet* cpp_packet = reinterpret_cast<const f3d::video_packet*>(packet);
  return cpp_packet->isKeyFrame() ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_video_encoder_get_available_encoders(char*** names, char*** descriptions, int* count)
{
  auto encoders = f3d::video_encoder::getAvailableEncoders();
  *count = static_cast<int>(encoders.size());
  *names = static_cast<char**>(malloc(sizeof(char*) * (*count)));
  *descriptions = static_cast<char**>(malloc(sizeof(char*) * (*count)));
  for (int i = 0; i < *count; ++i)
  {
    (*names)[i] = strdup(encoders[i].first.c_str());
    (*descriptions)[i] = strdup(encoders[i].second.c_str());
  }
}

//----------------------------------------------------------------------------
void f3d_video_encoder_free_available_encoders(char** names, char** descriptions, int count)
{
  for (int i = 0; i < count; ++i)
  {
    free(names[i]);
    free(descriptions[i]);
  }
  free(names);
  free(descriptions);
}

//----------------------------------------------------------------------------
f3d_video_encoder_t* f3d_video_encoder_new(const f3d_video_encoder_params_t* params)
{
  if (!params)
  {
    f3d::log::warn("Video encoder params is null, cannot create video encoder");
    return nullptr;
  }

  try
  {
    f3d::video_encoder::params cppParams;
    cppParams.Codec = static_cast<f3d::video_encoder::codec>(params->codec);
    cppParams.ExplicitCodecName = params->explicit_codec_name ? params->explicit_codec_name : "";
    cppParams.Width = params->width;
    cppParams.Height = params->height;
    cppParams.FrameRate = params->frame_rate;
    cppParams.Bitrate = params->bitrate;
    cppParams.LowLatency = params->low_latency != 0;

    return new f3d_video_encoder_t(f3d::video_encoder::create(cppParams));
  }
  catch (const f3d::video_encoder::codec_exception& e)
  {
    f3d::log::error("Cannot create video encoder: ", e.what());
    return nullptr;
  }
}

//----------------------------------------------------------------------------
void f3d_video_encoder_delete(f3d_video_encoder_t* encoder)
{
  if (!encoder)
  {
    f3d::log::warn("Video encoder is null, cannot delete video encoder");
    return;
  }

  delete reinterpret_cast<f3d_video_encoder_t*>(encoder);
}

//----------------------------------------------------------------------------
int f3d_video_encoder_get_width(const f3d_video_encoder_t* encoder)
{
  if (!encoder)
  {
    return 0;
  }

  return reinterpret_cast<const f3d_video_encoder_t*>(encoder)->impl->getWidth();
}

//----------------------------------------------------------------------------
int f3d_video_encoder_get_height(const f3d_video_encoder_t* encoder)
{
  if (!encoder)
  {
    return 0;
  }

  return reinterpret_cast<const f3d_video_encoder_t*>(encoder)->impl->getHeight();
}

//----------------------------------------------------------------------------
void f3d_video_encoder_listen(
  f3d_video_encoder_t* encoder, f3d_video_packet_callback_t callback, void* user_data)
{
  if (!encoder || !callback)
  {
    return;
  }

  f3d_video_encoder_t* cpp_encoder = reinterpret_cast<f3d_video_encoder_t*>(encoder);
  cpp_encoder->impl->listen([=](const std::shared_ptr<f3d::video_packet>& packet)
    { callback(reinterpret_cast<const f3d_video_packet_t*>(packet.get()), user_data); });
}

//----------------------------------------------------------------------------
int f3d_video_encoder_submit(f3d_video_encoder_t* encoder, f3d_video_frame_t* frame)
{
  if (!encoder || !frame)
  {
    return 0;
  }

  f3d_video_encoder_t* cpp_encoder = reinterpret_cast<f3d_video_encoder_t*>(encoder);
  f3d_video_frame_t* cpp_frame = reinterpret_cast<f3d_video_frame_t*>(frame);

  try
  {
    return cpp_encoder->impl->submit(cpp_frame->impl) ? 1 : 0;
  }
  catch (const f3d::video_encoder::transport_exception& e)
  {
    f3d::log::error("Cannot submit video frame: ", e.what());
    return -1;
  }
}

//----------------------------------------------------------------------------
void f3d_video_encoder_flush(f3d_video_encoder_t* encoder)
{
  if (!encoder)
  {
    return;
  }

  reinterpret_cast<f3d_video_encoder_t*>(encoder)->impl->flush();
}

//----------------------------------------------------------------------------
f3d_video_frame_t* f3d_window_get_video_frame(f3d_window_t* window)
{
  if (!window)
  {
    return nullptr;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  return new f3d_video_frame_t(cpp_window->getVideoFrame());
}
