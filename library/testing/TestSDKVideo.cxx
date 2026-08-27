#include "PseudoUnitTest.h"

#include <engine.h>
#include <video_encoder.h>

#include <thread>

int TestSDKVideo([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  // testing all generic encoders
  for (auto codec : { f3d::video_encoder::codec::H264, f3d::video_encoder::codec::HEVC,
         f3d::video_encoder::codec::VP8, f3d::video_encoder::codec::VP9,
         f3d::video_encoder::codec::AV1 })
  {
    try
    {
      std::ignore = f3d::video_encoder::create({ .Codec = codec, .Width = 300, .Height = 300 });
    }
    catch (f3d::video_encoder::codec_exception&)
    {
      // if the codec is not available, it's fine
    }
  }

  // list encoders and test them
  auto encoders = f3d::video_encoder::getAvailableEncoders();
  for (const auto& [name, description] : encoders)
  {
    f3d::log::info("Available encoder: ", name, "(", description, ")");

    try
    {
      std::ignore = f3d::video_encoder::create({ .Codec = f3d::video_encoder::codec::EXPLICIT,
        .ExplicitCodecName = name,
        .Width = 300,
        .Height = 300 });
    }
    catch (f3d::video_encoder::codec_exception&)
    {
      // if the codec is not available, it's fine
    }
  }

  // explicit with empty codec name should fail
  test.expect<f3d::video_encoder::codec_exception>("Explicit codec name required", [&]() {
    std::ignore = f3d::video_encoder::create(
      { .Codec = f3d::video_encoder::codec::EXPLICIT, .Width = 300, .Height = 300 });
  });

  // explicit with invalid codec name should fail
  test.expect<f3d::video_encoder::codec_exception>("Invalid explicit codec name", [&]() {
    std::ignore = f3d::video_encoder::create({ .Codec = f3d::video_encoder::codec::EXPLICIT,
      .ExplicitCodecName = "invalid_codec",
      .Width = 300,
      .Height = 300 });
  });

  // testing encoder initialization failure
  // width and height are 0, should fail
  test.expect<f3d::video_encoder::codec_exception>("video encoder initialization failure",
    [&]() { std::ignore = f3d::video_encoder::create({}); });

  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();

  test.expect<f3d::video_frame::invalid_frame_exception>("Uninitialized window",
    [&]() { std::shared_ptr<f3d::video_frame> uninitFrame = win.getVideoFrame(); });

  win.setSize(100, 101);
  test.expect<f3d::video_frame::invalid_frame_exception>("Size not even",
    [&]() { std::shared_ptr<f3d::video_frame> oddDimFrame = win.getVideoFrame(); });

  win.setSize(1500000000, 1500000000);
  test.expect<f3d::video_frame::invalid_frame_exception>("Size too big",
    [&]() { std::shared_ptr<f3d::video_frame> wrongDimFrame = win.getVideoFrame(); });

  // use auto for the actual test
  auto encoder = f3d::video_encoder::create(
    { .Codec = f3d::video_encoder::codec::H264, .Width = 300, .Height = 300, .LowLatency = true });

  test("video encoder width", encoder->getWidth() == 300);
  test("video encoder height", encoder->getHeight() == 300);

  encoder->listen([](const std::shared_ptr<f3d::video_packet>&) {});
  encoder->listen(nullptr);

  // get a 100x100 frame but not submit it to the 300x300 encoder
  // it's there to cover the change of resolution in the frame capture class
  win.setSize(100, 100).render();
  std::shared_ptr<f3d::video_frame> frame = win.getVideoFrame();

  // submit the video frame to the encoder
  win.setSize(300, 300).render();

  frame = win.getVideoFrame();
  frame->setTimestamp(41);
  encoder->submit(frame);

  frame = win.getVideoFrame();
  frame->setTimestamp(42);

  while (encoder->submit(frame))
  {
    // submit the frame until the encoder queue is full
  }

  // add proper listener
  int64_t packetTS = 0;
  encoder->listen([&](const std::shared_ptr<f3d::video_packet>& packet) {
    packetTS = packet->getTimestamp();
    packet->isKeyFrame(); // coverage
    test("video packet size", packet->getPacketSize() > 0);
    test("video packet data", packet->getPacketData() != nullptr);
  });

  // Make sure the listener is ready before flushing
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // flush (block until all packets are drained)
  encoder->flush();

  // submit after flush
  test.expect<f3d::video_encoder::transport_exception>(
    "Submit after flush", [&]() { encoder->submit(frame); });

  test("video packet timestamp", packetTS == 42);

  return test.result();
}
