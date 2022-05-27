#include <image.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

int TestSDKImage(int argc, char* argv[])
{
  constexpr unsigned int width = 64;
  constexpr unsigned int height = 64;
  constexpr unsigned int channels = 3;

  std::vector<unsigned char> pixels(width * height * channels);

  // fill with deterministic random values
  auto rand = std::bind(std::uniform_int_distribution<>(0, 255), std::mt19937());
  std::generate(std::begin(pixels), std::end(pixels), rand);

  f3d::image generated;
  generated.setResolution(width, height).setChannelCount(channels).setData(pixels.data());
  generated.save(std::string(argv[2]) + "TestSDKImage.png");

  f3d::image baseline(std::string(argv[1]) + "/baselines/TestSDKImage.png");

  if (baseline.getWidth() != width || baseline.getHeight() != height || baseline.getChannelCount() != channels)
  {
    std::cerr << "Baseline has wrong dimensions" << std::endl;
    return EXIT_FAILURE;
  }

  if (baseline.getChannelCount() != channels)
  {
    std::cerr << "Baseline has wrong number of channels" << std::endl;
    return EXIT_FAILURE;
  }

  if (baseline.getData() == nullptr)
  {
    std::cerr << "Baseline has no data" << std::endl;
    return EXIT_FAILURE;
  }

  return generated == baseline ? EXIT_SUCCESS : EXIT_FAILURE;
}
