#include "PseudoUnitTest.h"

#include <image.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <sstream>

int TestSDKImage(int argc, char* argv[])
{
  PseudoUnitTest test;

  const std::string testingDir(argv[1]);
  const std::string tmpDir(argv[2]);

  // check supported formats
  std::vector<std::string> formats = f3d::image::getSupportedFormats();
  test("supported formats PNG", std::find(formats.begin(), formats.end(), ".png") != formats.end());
#if F3D_MODULE_EXR
  test("supported formats EXR", std::find(formats.begin(), formats.end(), ".exr") != formats.end());
#endif

  constexpr unsigned int width = 64;
  constexpr unsigned int height = 64;
  constexpr unsigned int channels = 3;

  // fill with deterministic random values
  // do not use std::uniform_int_distribution, it's not giving the same result on different
  // platforms
  std::mt19937 randGenerator;

  f3d::image generated(width, height, channels);
  std::vector<uint8_t> pixels(width * height * channels);
  std::generate(std::begin(pixels), std::end(pixels), [&]() { return randGenerator() % 256; });
  generated.setContent(pixels.data());

  f3d::image generated16(width, height, channels, f3d::image::ChannelType::SHORT);
  std::vector<uint16_t> pixels16(width * height * channels);
  std::generate(
    std::begin(pixels16), std::end(pixels16), [&]() { return randGenerator() % 65536; });
  generated16.setContent(pixels16.data());

  std::uniform_real_distribution<float> dist(
    std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
  f3d::image generated32(width, height, channels, f3d::image::ChannelType::FLOAT);
  std::vector<float> pixels32(width * height * channels);
  std::generate(std::begin(pixels32), std::end(pixels32), [&]() { return dist(randGenerator); });
  generated32.setContent(pixels32.data());

  // test save in different formats and different types
  generated.save(tmpDir + "/TestSDKImage.png");
  generated.save(tmpDir + "/TestSDKImage.jpg", f3d::image::SaveFormat::JPG);
  generated.save(tmpDir + "/TestSDKImage.bmp", f3d::image::SaveFormat::BMP);
  generated.save(tmpDir + "/TestSDKImage.tif", f3d::image::SaveFormat::TIF);
  generated16.save(tmpDir + "/TestSDKImage16.png");
  generated16.save(tmpDir + "/TestSDKImage16.tif", f3d::image::SaveFormat::TIF);
  generated32.save(tmpDir + "/TestSDKImage32.tif", f3d::image::SaveFormat::TIF);

  // test saveBuffer in different formats
  std::vector<unsigned char> bufferPNG = generated.saveBuffer();
  test("generated buffer not empty", bufferPNG.size() != 0);

  std::vector<unsigned char> bufferJPG = generated.saveBuffer(f3d::image::SaveFormat::JPG);
  test("generated JPG buffer not empty", bufferJPG.size() != 0);

  test.expect<f3d::image::write_exception>("save incompatible buffer to TIF format",
    [&]() { std::ignore = generated.saveBuffer(f3d::image::SaveFormat::TIF); });

  std::vector<unsigned char> bufferBMP = generated.saveBuffer(f3d::image::SaveFormat::BMP);
  test("generated BMP buffer not empty", bufferBMP.size() != 0);

  // test constructor with different channel sizes
  f3d::image img16(width, height, channels, f3d::image::ChannelType::SHORT);
  f3d::image img32(width, height, channels, f3d::image::ChannelType::FLOAT);

  // test exceptions
  test.expect<f3d::image::write_exception>("save incompatible buffer to BMP format",
    [&]() { std::ignore = img16.saveBuffer(f3d::image::SaveFormat::BMP); });
  test.expect<f3d::image::write_exception>("save incompatible buffer to PNG format",
    [&]() { std::ignore = img32.saveBuffer(f3d::image::SaveFormat::PNG); });

  f3d::image img2Ch(4, 4, 2);
  f3d::image img5Ch(4, 4, 5);
  test.expect<f3d::image::write_exception>("save incompatible channel count to BMP format",
    [&]() { std::ignore = img5Ch.saveBuffer(f3d::image::SaveFormat::BMP); });
  test.expect<f3d::image::write_exception>("save incompatible channel count to JPG format",
    [&]() { std::ignore = img2Ch.saveBuffer(f3d::image::SaveFormat::JPG); });
  test.expect<f3d::image::write_exception>("save image to invalid path",
    [&]() { img2Ch.save("/" + std::string(257, 'x') + "/file.ext"); });
  test.expect<f3d::image::write_exception>("save image to invalid filename",
    [&]() { img2Ch.save(testingDir + std::string(257, 'x') + ".ext"); });

  // check 16-bits image code paths
  f3d::image shortImg(testingDir + "/data/16bit.png");
  test("check 16-bits image channel type",
    shortImg.getChannelType() == f3d::image::ChannelType::SHORT);
  test("check 16-bits image channel type size", shortImg.getChannelTypeSize() == 2);

  // check reading a 32-bits image
  f3d::image hdrImg(testingDir + "/data/palermo_park_1k.hdr");
  test("check 32-bits HDR image channel type",
    hdrImg.getChannelType() == f3d::image::ChannelType::FLOAT);
  test("check 32-bits HDR image channel type size", hdrImg.getChannelTypeSize() == 4);
  hdrImg.save(tmpDir + "/TestSDKImage32hdr.tif", f3d::image::SaveFormat::TIF);

#if F3D_MODULE_EXR
  // check reading EXR
  f3d::image exrImg(testingDir + "/data/kloofendal_43d_clear_1k.exr");
  test("check 32-bits EXR image channel type",
    exrImg.getChannelType() == f3d::image::ChannelType::FLOAT);
#endif

  // check reading invalid image
  test.expect<f3d::image::read_exception>(
    "read invalid image", [&]() { f3d::image invalidImg(testingDir + "/data/invalid.png"); });

  // check reading inexistent image, do not create a "/dummy/folder/img.png"
  test.expect<f3d::image::read_exception>(
    "read image from incorrect path", [&]() { f3d::image img("/dummy/folder/img.png"); });

  // check reading image with invalid path
  test.expect<f3d::image::read_exception>("read image from invalid path",
    [&]() { f3d::image img("/" + std::string(257, 'x') + "/file.ext"); });

  // check generated image with baseline
  test(
    "check generated image size", generated.getWidth() == width && generated.getHeight() == height);
  test("check generated image channel count", generated.getChannelCount() == channels);
  test("check generated image channel type",
    generated.getChannelType() == f3d::image::ChannelType::BYTE);
  test("check generated image not empty", generated.getContent() != nullptr);

  // XXX: PseudoUnitTest could be improved for native image testing
  f3d::image baseline(testingDir + "/baselines/TestSDKImage.png");
  if (generated != baseline)
  {
    std::cerr << "Generated image is different from the png baseline: "
              << generated.compare(baseline) << std::endl;
    return EXIT_FAILURE;
  }

  // XXX: enable following code once https://github.com/f3d-app/f3d/issues/1558 is fixed
  /*
  f3d::image baselineTIF(testingDir + "/baselines/TestSDKImage.tif");
  if (generated != baselineTIF)
  {
    std::cerr << "Generated image is different from the tif baseline: "
              << generated.compare(baselineTIF) << std::endl;
    return EXIT_FAILURE;
  }*/

// Remove this once VTK 9.3 support is removed
#ifdef F3D_SSIM_COMPARE
  // check generated short image with baseline
  test("check generated short image size",
    generated16.getWidth() == width && generated16.getHeight() == height);
  test("check generated short image channel count", generated16.getChannelCount() == channels);
  test("check generated short image channel type",
    generated16.getChannelType() == f3d::image::ChannelType::SHORT);
  test("check generated short image not empty", generated16.getContent() != nullptr);

  f3d::image baseline16(testingDir + "/baselines/TestSDKImage16.png");
  if (generated16 != baseline16)
  {
    std::cerr << "generated short image is different from the baseline: "
              << generated16.compare(baseline16) << std::endl;
    return EXIT_FAILURE;
  }

  // XXX: enable following code once https://github.com/f3d-app/f3d/issues/1558 is fixed
  /*
  f3d::image baseline16TIF(testingDir + "/baselines/TestSDKImage16.tif");
  if (generated16 != baseline16TIF)
  {
    std::cerr << "generated short image is different from the TIF baseline: "
              << generated16.compare(baseline16TIF) << std::endl;
    return EXIT_FAILURE;
  }*/

  // check generated float image with baseline
  // XXX: Uncomment once https://github.com/f3d-app/f3d/issues/1558 is fixed
  // f3d::image baseline32(testingDir + "/baselines/TestSDKImage32.tif");
  f3d::image baseline32 = generated32;

  test("check generated float image size",
    generated32.getWidth() == width && generated32.getHeight() == height);
  test("check generated float image channel count", generated32.getChannelCount() == channels);
  test("check generated float image channel type",
    generated32.getChannelType() == f3d::image::ChannelType::FLOAT);
  test("check generated float image not empty", generated32.getContent() != nullptr);

  if (generated32 != baseline32)
  {
    std::cerr << "generated float image is different from the baseline: "
              << generated32.compare(baseline32) << std::endl;
    return EXIT_FAILURE;
  }
#endif // F3D_SSIM_COMPARE

  // test operators
  f3d::image imgCopy = generated; // copy constructor
  test("check copy constructor", imgCopy == generated);

  imgCopy = baseline; // copy assignment
  test("check copy assignment", imgCopy == baseline);

  f3d::image imgMove = std::move(imgCopy); // move constructor
  test("check move constructor", imgMove == baseline);

  imgCopy = std::move(imgMove); // move assignment
  test("check move assignment", imgCopy == baseline);

  // test toTerminalText
  {
    test.expect<f3d::image::write_exception>("invalid toTerminalText with BYTE",
      [&]() { std::ignore = f3d::image(3, 3, 1, f3d::image::ChannelType::BYTE).toTerminalText(); });
    test.expect<f3d::image::write_exception>("invalid toTerminalText with SHORT", [&]() {
      std::ignore = f3d::image(3, 3, 4, f3d::image::ChannelType::SHORT).toTerminalText();
    });

    const auto fileToString = [](const std::string& path) {
      std::ifstream file(path);
      std::stringstream ss;
      ss << file.rdbuf();
      return ss.str();
    };

    test("toTerminalText with RGB image",
      f3d::image(testingDir + "/data/toTerminalText-rgb.png").toTerminalText() ==
        fileToString(testingDir + "/data/toTerminalText-rgb.txt"));
    test("toTerminalText with RGBA image",
      f3d::image(testingDir + "/data/toTerminalText-rgba.png").toTerminalText() ==
        fileToString(testingDir + "/data/toTerminalText-rgba.txt"));
  }

  // test metadata
  {

    f3d::image img(4, 2, 3);
    img.setMetadata("foo", "bar");
    img.setMetadata("hello", "world");
    test("check metadata", img.getMetadata("foo") == "bar" && img.getMetadata("hello") == "world");

    const std::vector<std::string> keys = img.allMetadata();
    test("check all metadata",
      std::set<std::string>(keys.begin(), keys.end()) == std::set<std::string>({ "foo", "hello" }));

    test.expect<f3d::image::metadata_exception>(
      "invalid get metadata", [&]() { std::ignore = img.getMetadata("baz"); });

    test.expect<f3d::image::metadata_exception>("remove and get metadata", [&]() {
      img.setMetadata("foo", "");           // empty value, should remove key
      std::ignore = img.getMetadata("foo"); // expected to throw
    });

    test(
      "check all metata after removal", img.allMetadata() == std::vector<std::string>({ "hello" }));
    img.setMetadata("foo", ""); // make sure removing twice is ok
  }

  {
    f3d::image img1(4, 2, 3);
    img1.setMetadata("foo", "bar");
    img1.setMetadata("hello", "world");
    img1.save(tmpDir + "/metadata.png");

    f3d::image img2(tmpDir + "/metadata.png");
    test("saving/loading file metadata",
      img2.getMetadata("foo") == "bar" && img2.getMetadata("hello") == "world");
  }

  {
    f3d::image img1(4, 2, 3);
    img1.setMetadata("foo", "bar");
    img1.setMetadata("hello", "world");
    {
      std::vector<unsigned char> buffer = img1.saveBuffer();
      std::ofstream outfile(tmpDir + "/metadata-buffer.png", std::ios::out | std::ios::binary);
      outfile.write((const char*)&buffer[0], buffer.size());
    }

    f3d::image img2(tmpDir + "/metadata-buffer.png");
    test("saving/loading buffer metadata",
      img2.getMetadata("foo") == "bar" && img2.getMetadata("hello") == "world");
  }

  // Test image::compare dedicated code paths
  test("compare images with different channel types", generated.compare(generated16) == 1.);

  f3d::image generatedCount(width, height, channels + 1);
  test("compare images with different channel count", generated.compare(generatedCount) == 1.);

  f3d::image generatedSize(width + 1, height, channels);
  test("compare images with different size", generated.compare(generatedSize) == 1.);

  f3d::image empty(0, 0, 0);
  test("compare empty images", empty.compare(empty) == 0.);
  test("compare with negative threshold", empty.compare(empty) == 1.);
  test("compare with threshold == 1", empty.compare(empty) == 1.);

  return test.result();
}
