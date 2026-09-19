#include <f3d/engine.h>
#include <f3d/video_encoder.h>

#include <thread>

namespace
{
// Parse optional thumbnail size:
//   argv[2]      -> square size (N x N)
//   argv[2,3]    -> width height
bool ParseSize(int argc, char** argv, int& width, int& height)
{
  // Default thumbnail size
  width = 512;
  height = 512;

  if (argc == 2)
  {
    // Just use the default
    return true;
  }

  try
  {
    if (argc == 3)
    {
      // Single integer: square thumbnail
      const int size = std::stoi(argv[2]);
      if (size <= 0)
      {
        return false;
      }
      width = size;
      height = size;
      return true;
    }
    if (argc == 4)
    {
      width = std::stoi(argv[2]);
      height = std::stoi(argv[3]);
      if (width <= 0 || height <= 0)
      {
        return false;
      }
      return true;
    }
  }
  catch (const std::exception&)
  {
    return false;
  }

  return false;
}

void PrintUsage(const char* exe)
{
  std::cerr << "Usage:\n"
            << "  " << exe << " <input-model> [size]\n"
            << "  " << exe << " <input-model> <width> <height>\n\n"
            << "Examples:\n"
            << "  " << exe << " model.glb        # 512x512 (default)\n"
            << "  " << exe << " model.glb 256    # 256x256\n"
            << "  " << exe << " model.glb 512 256\n"
            << "width and height must be even numbers.\n";
}
}

int main(int argc, char** argv)
{
  try
  {
    if (argc < 2 || argc > 4)
    {
      ::PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    const std::string inputFile = argv[1];

    // Set log level to info and force output to stderr
    // This is important to ensure that the video stream is not corrupted by log messages
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::INFO, true);

    int width = 0;
    int height = 0;
    if (!::ParseSize(argc, argv, width, height))
    {
      f3d::log::error("Invalid video size.");
      ::PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    if (width % 2 != 0 || height % 2 != 0)
    {
      f3d::log::error("Video size must be even.");
      ::PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    // Load static/native plugins
    f3d::engine::autoloadPlugins();

    // Create an offscreen engine and add input file
    f3d::engine eng = f3d::engine::create(true);
    eng.getScene().add(inputFile);

    // Render offscreen to an image
    f3d::window& win = eng.getWindow();
    win.setSize(width, height);

    constexpr double framerate = 30;

    f3d::video_encoder::params params;
    params.Codec = f3d::video_encoder::codec::H264;
    params.Width = width;
    params.Height = height;
    params.FrameRate = framerate;
    params.Bitrate = 5.0;

    auto encoder = f3d::video_encoder::create(params);

    encoder->listen(
      [](const std::shared_ptr<f3d::video_packet>& packet)
      {
        // write packets to stdout
        // it can be piped to ffmpeg for example to create a video file
        // e.g.
        // ./offscreen-video model.glb 512 512 | ffmpeg -f h264 -i - output.mp4
        // ./offscreen-video model.glb 512 512 | ffplay -f h264 -
        std::cout.write(
          reinterpret_cast<const char*>(packet->getPacketData()), packet->getPacketSize());
      });

    auto [startTime, endTime] = eng.getScene().animationTimeRange();

    int frameCount = static_cast<int>((endTime - startTime) * framerate) + 1;

    for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex)
    {
      const double t = startTime + static_cast<double>(frameIndex) / framerate;

      eng.getScene().loadAnimationTime(t);
      win.render();

      // submit the video frame to the encoder
      auto frame = win.getVideoFrame();
      frame->setTimestamp(frameIndex);

      while (!encoder->submit(frame))
      {
        // wait for the encoder to be ready to accept new frames
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }

    // flush the encoder to ensure all frames are processed and packets are sent
    encoder->flush();

    return EXIT_SUCCESS;
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("F3D video example encountered an unexpected exception:");
    f3d::log::error(ex.what());
    return EXIT_FAILURE;
  }
}
