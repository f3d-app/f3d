#include "PseudoUnitTest.h"

#include <image.h>

#include <algorithm>
#include <random>

int TestImageStream([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
  PseudoUnitTest test;

  constexpr unsigned int width = 64;
  constexpr unsigned int height = 64;
  constexpr unsigned int channels = 3;

  // fill with deterministic random values
  // do not use std::uniform_int_distribution, it's not giving the same result on different
  // platforms
  std::mt19937 randGenerator;

  f3d::image generated(width, height, channels);
  std::vector<uint8_t> pixels(width * height * channels);
  std::ranges::generate(pixels, [&]() { return static_cast<uint8_t>(randGenerator() % 256); });
  generated.setContent(pixels.data());

  // check reading stream
  std::vector<unsigned char> generatedBuffer = generated.saveBuffer();
  std::byte* bufferData = reinterpret_cast<std::byte*>(generatedBuffer.data());
  f3d::image bufferImage(bufferData, generatedBuffer.size());
  test("check loading stream from image reader", generated.compare(bufferImage), 0.0);

  // check reading inexistent/null stream
  test.expect<f3d::image::read_exception>(
    "read image from invalid/null stream", [&]() { f3d::image nullImgStream(nullptr, 10); });

  // check reading invalid stream
  std::vector<unsigned char> invalidBuffer = { 0, 1, 2, 3, 4, 5 };
  std::byte* invalidBufferData = reinterpret_cast<std::byte*>(invalidBuffer.data());
  test.expect<f3d::image::read_exception>("read image from invalid stream",
    [&]() { f3d::image invalidImgStream(invalidBufferData, 10); });

  return test.result();
}
