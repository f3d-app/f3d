#include <engine_c_api.h>
#include <image_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

void packet_callback(const f3d_video_packet_t* packet, void* user_data)
{
  size_t packet_size = f3d_video_packet_get_packet_size(packet);
  const unsigned char* packet_data = f3d_video_packet_get_packet_data(packet);
  *(int*)user_data = f3d_video_packet_get_timestamp(packet);
}

int test_video()
{
  f3d_engine_t* engine = f3d_engine_create(1);
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (!window)
  {
    puts("[ERROR] Failed to get window");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_window_render(window);

  // get available encoders
  int encoder_count = 0;
  char** encoder_names = NULL;
  char** encoder_descriptions = NULL;
  f3d_video_encoder_get_available_encoders(&encoder_names, &encoder_descriptions, &encoder_count);

  for (int i = 0; i < encoder_count; ++i)
  {
    printf("Encoder %d: %s - %s\n", i, encoder_names[i], encoder_descriptions[i]);
  }

  f3d_video_encoder_free_available_encoders(encoder_names, encoder_descriptions, encoder_count);

  // video
  f3d_video_encoder_params_t params;
  params.width = f3d_window_get_width(window);
  params.height = f3d_window_get_height(window);
  params.codec = F3D_VIDEO_ENCODER_CODEC_H264;
  params.explicit_codec_name = NULL;
  params.frame_rate = 30.0;
  params.bitrate = 5.0;
  params.low_latency = 0;

  f3d_video_encoder_t* encoder = f3d_video_encoder_new(&params);
  if (!encoder)
  {
    puts("[ERROR] Failed to create video encoder");
    f3d_engine_delete(engine);
    return 1;
  }

  int ts = 0;
  f3d_video_encoder_listen(encoder, packet_callback, &ts);

  f3d_video_frame_t* video_frame = f3d_window_get_video_frame(window);
  if (!video_frame)
  {
    puts("[ERROR] Failed to get video frame");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_video_frame_set_timestamp(video_frame, 42);

  f3d_video_encoder_submit(encoder, video_frame);
  f3d_video_frame_delete(video_frame);

  f3d_video_encoder_flush(encoder);
  f3d_video_encoder_delete(encoder);

  if (ts != 42)
  {
    puts("[ERROR] Video packet callback was not called with the expected timestamp");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_engine_delete(engine);
  return 0;
}
