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
  try
  {
    generated.save("/dummy/folder/img.png");

    std::cerr << "An exception has not been thrown when saving to an incorrect path" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::image::write_exception&)
  {
  }

  try
  {
    f3d::image("/dummy/folder/img.png");

    std::cerr << "An exception has not been thrown when reading an incorrect path" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::image::read_exception&)
  {
  }

  f3d::image baseline(std::string(argv[1]) + "/baselines/TestSDKImage.png");

  if (generated.getWidth() != width || generated.getHeight() != height)
  {
    std::cerr << "Image has wrong dimensions" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated.getChannelCount() != channels)
  {
    std::cerr << "Image has wrong number of channels" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated.getData() == nullptr)
  {
    std::cerr << "Image has no data" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated != baseline)
  {
    std::cerr << "Generated image is different from the baseline" << std::endl;
    return EXIT_FAILURE;
  }

  // test operators
  f3d::image imgCopy = generated; // copy constructor

  if (imgCopy != generated)
  {
    std::cerr << "Copy constructor failed" << std::endl;
    return EXIT_FAILURE;
  }

  imgCopy = baseline; // copy assignment

  if (imgCopy != baseline)
  {
    std::cerr << "Copy assignment failed" << std::endl;
    return EXIT_FAILURE;
  }

  f3d::image imgMove = std::move(imgCopy); // move constructor

  if (imgMove != baseline)
  {
    std::cerr << "Move constructor failed" << std::endl;
    return EXIT_FAILURE;
  }

  imgCopy = std::move(imgMove); // move assignment

  if (imgCopy != baseline)
  {
    std::cerr << "Move assignment failed" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
