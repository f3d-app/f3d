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
  // do not use std::uniform_int_distribution, it's not giving the same result on different
  // platforms
  std::mt19937 rand_generator;
  std::generate(std::begin(pixels), std::end(pixels), [&]() { return rand_generator() % 256; });

  f3d::image generated;
  generated.setResolution(width, height).setChannelCount(channels).setData(pixels.data());
  generated.save(std::string(argv[2]) + "TestSDKImage.png");

  // test exceptions
  int nbCatch = 0;

  try
  {
    generated.save("/dummy/folder/img.png");
  }
  catch (const f3d::image::exception&)
  {
    nbCatch++;
  }

  try
  {
    f3d::image("/dummy/folder/img.png");
  }
  catch (const f3d::image::exception&)
  {
    nbCatch++;
  }

  if (nbCatch != 2)
  {
    std::cerr << "An exception has not been thrown" << std::endl;
    return EXIT_FAILURE;
  }

  f3d::image baseline(std::string(argv[1]) + "/baselines/TestSDKImage.png");

  if (baseline.getWidth() != width || baseline.getHeight() != height ||
    baseline.getChannelCount() != channels)
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

  return generated != baseline ? EXIT_FAILURE : EXIT_SUCCESS;
}
