#include <image.h>
#include <log.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

int TestSDKImage(int argc, char* argv[])
{
  const std::string testingDir(argv[1]);
  const std::string tmpDir(argv[2]);

  // check supported formats
  std::vector<std::string> formats = f3d::image::getSupportedFormats();

  if (std::find(formats.begin(), formats.end(), ".png") == formats.end())
  {
    std::cerr << "PNG is not in the list of supported files" << std::endl;
    return EXIT_FAILURE;
  }

#if F3D_MODULE_EXR
  if (std::find(formats.begin(), formats.end(), ".exr") == formats.end())
  {
    std::cerr << "EXR is not in the list of supported files" << std::endl;
    return EXIT_FAILURE;
  }
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
  std::generate(std::begin(pixels16), std::end(pixels16), [&]() { return randGenerator() % 65536; });
  generated16.setContent(pixels16.data());

  std::uniform_real_distribution<> dist(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
  f3d::image generated32(width, height, channels, f3d::image::ChannelType::FLOAT);
  std::vector<float> pixels32(width * height * channels);
  std::generate(std::begin(pixels32), std::end(pixels32), [&]() { return dist(randGenerator); });
  generated32.setContent(pixels32.data());

  // test save in different formats and different types
  generated.save(tmpDir + "/TestSDKImage.png");
  generated16.save(tmpDir + "/TestSDKImage16.png");
  generated32.save(tmpDir + "/TestSDKImage32.tif", f3d::image::SaveFormat::TIF);
  generated.save(tmpDir + "/TestSDKImage.jpg", f3d::image::SaveFormat::JPG);
  generated.save(tmpDir + "/TestSDKImage.tif", f3d::image::SaveFormat::TIF);
  generated.save(tmpDir + "/TestSDKImage.bmp", f3d::image::SaveFormat::BMP);

  // test saveBuffer in different formats
  std::vector<unsigned char> bufferPNG = generated.saveBuffer();
  if (bufferPNG.size() == 0)
  {
    std::cerr << "PNG buffer empty" << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<unsigned char> bufferJPG = generated.saveBuffer(f3d::image::SaveFormat::JPG);
  if (bufferJPG.size() == 0)
  {
    std::cerr << "JPG buffer empty" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    generated.saveBuffer(f3d::image::SaveFormat::TIF);
    std::cerr << "An exception has not been thrown when saving buffer to TIF format" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::image::write_exception&)
  {
  }

  std::vector<unsigned char> bufferBMP = generated.saveBuffer(f3d::image::SaveFormat::BMP);
  if (bufferBMP.size() == 0)
  {
    std::cerr << "BMP buffer empty" << std::endl;
    return EXIT_FAILURE;
  }

  // test constructor with different channel sizes
  f3d::image img16(width, height, channels, f3d::image::ChannelType::SHORT);
  f3d::image img32(width, height, channels, f3d::image::ChannelType::FLOAT);

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
    f3d::image img("/dummy/folder/img.png");

    std::cerr << "An exception has not been thrown when reading an incorrect path" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::image::read_exception&)
  {
  }

  // check 16-bits image code paths
  f3d::image shortImg(testingDir + "/data/16bit.png");

  if (shortImg.getChannelType() != f3d::image::ChannelType::SHORT)
  {
    std::cerr << "Cannot read a 16-bits image type" << std::endl;
    return EXIT_FAILURE;
  }

  if (shortImg.getChannelTypeSize() != 2)
  {
    std::cerr << "Cannot read a 16-bits image type size" << std::endl;
    return EXIT_FAILURE;
  }

  // check reading a 32-bits image
  f3d::image hdrImg(testingDir + "/data/palermo_park_1k.hdr");

  if (hdrImg.getChannelType() != f3d::image::ChannelType::FLOAT)
  {
    std::cerr << "Cannot read a HDR 32-bits image" << std::endl;
    return EXIT_FAILURE;
  }

  if (hdrImg.getChannelTypeSize() != 4)
  {
    std::cerr << "Cannot read a HDR 32-bits image type size" << std::endl;
    return EXIT_FAILURE;
  }
  hdrImg.save(tmpDir + "/TestSDKImage32hdr.tif", f3d::image::SaveFormat::TIF);

#if F3D_MODULE_EXR
  // check reading EXR
  f3d::image exrImg(testingDir + "/data/kloofendal_43d_clear_1k.exr");

  if (exrImg.getChannelType() != f3d::image::ChannelType::FLOAT)
  {
    std::cerr << "Cannot read a EXR 32-bits image" << std::endl;
    return EXIT_FAILURE;
  }
#endif

  // check reading invalid image
  try
  {
    f3d::image invalidImg(testingDir + "/data/invalid.png");

    std::cerr << "An exception has not been thrown when reading an invalid file" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::image::read_exception&)
  {
  }

  // check generated image with baseline
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

  if (generated.getChannelType() != f3d::image::ChannelType::BYTE)
  {
    std::cerr << "Image has wrong channel size" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated.getContent() == nullptr)
  {
    std::cerr << "Image has no data" << std::endl;
    return EXIT_FAILURE;
  }

  f3d::image baseline(testingDir + "/baselines/TestSDKImage.png");
  if (generated != baseline)
  {
    double error;
    generated.compare(baseline, 0, error);

    std::cerr << "Generated image is different from the png baseline: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // XXX: enable following code once TODO is fixed
  /*
  f3d::image baselineJPG(testingDir + "/baselines/TestSDKImage.jpg");
  if (generated != baselineJPG)
  {
    double error;
    generated.compare(baselineJPG, 0, error);

    std::cerr << "Generated image is different from the jpg baseline: " << error << std::endl;
    return EXIT_FAILURE;
  }*/

  // XXX: enable following code once https://github.com/f3d-app/f3d/issues/1558 is fixed
  /*
  f3d::image baselineTIF(testingDir + "/baselines/TestSDKImage.tif");
  if (generated != baselineTIF)
  {
    double error;
    baselineTIF2.compare(baselineTIF, 0, error);

    std::cerr << "Generated image is different from the tif baseline: " << error << std::endl;
    return EXIT_FAILURE;
  }*/

// Remove this once VTK 9.3 support is removed
//#if F3D_SSIM_COMPARE
  // check generated short image with baseline
  f3d::image baseline16(testingDir + "/baselines/TestSDKImage16.png");
  if (generated16.getWidth() != width || generated16.getHeight() != height)
  {
    std::cerr << "Short image has wrong dimensions" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated16.getChannelCount() != channels)
  {
    std::cerr << "Short image has wrong number of channels" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated16.getChannelType() != f3d::image::ChannelType::SHORT)
  {
    std::cerr << "Short image has wrong channel size" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated16.getContent() == nullptr)
  {
    std::cerr << "Short image has no data" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated16 != baseline16)
  {
    double error;
    generated16.compare(baseline16, 0, error);

    std::cerr << "generated short image is different from the baseline: " << error << std::endl;
    return EXIT_FAILURE;
  }

  // check generated float image with baseline
  // XXX: Uncomment once https://github.com/f3d-app/f3d/issues/1558 is fixed
  //f3d::image baseline32(testingDir + "/baselines/TestSDKImage32.tif");
  f3d::image baseline32 = generated32;
  if (generated32.getWidth() != width || generated32.getHeight() != height)
  {
    std::cerr << "Float image has wrong dimensions" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated32.getChannelCount() != channels)
  {
    std::cerr << "Float image has wrong number of channels" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated32.getChannelType() != f3d::image::ChannelType::FLOAT)
  {
    std::cerr << "Float image has wrong channel size" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated32.getContent() == nullptr)
  {
    std::cerr << "Float image has no data" << std::endl;
    return EXIT_FAILURE;
  }

  if (generated32 != baseline32)
  {
    double error;
    generated32.compare(baseline32, 0, error);

    std::cerr << "generated float image is different from the baseline: " << error << std::endl;
    return EXIT_FAILURE;
  }
//#endif

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

  {
    try
    {
      f3d::image(3, 3, 1, f3d::image::ChannelType::BYTE).toTerminalText();
      return EXIT_FAILURE; // expected to throw (wrong channel count)
    }
    catch (const std::invalid_argument& e)
    {
    }

    try
    {
      f3d::image(3, 3, 4, f3d::image::ChannelType::SHORT).toTerminalText();
      return EXIT_FAILURE; // expected to throw (wrong channel type)
    }
    catch (const std::invalid_argument& e)
    {
    }

    const auto fileToString = [](const std::string& path)
    {
      std::ifstream file(path);
      std::stringstream ss;
      ss << file.rdbuf();
      return ss.str();
    };

    if (f3d::image(testingDir + "/data/toTerminalText-rgb.png").toTerminalText() !=
      fileToString(testingDir + "/data/toTerminalText-rgb.txt"))
    {
      std::cerr << "toTerminalText() (RGB image) failed" << std::endl;
      return EXIT_FAILURE;
    }

    if (f3d::image(testingDir + "/data/toTerminalText-rgba.png").toTerminalText() !=
      fileToString(testingDir + "/data/toTerminalText-rgba.txt"))
    {
      std::cerr << "toTerminalText() (RGBA image) failed" << std::endl;
      return EXIT_FAILURE;
    }
  }

  {
    f3d::image img(4, 2, 3);
    img.setMetadata("foo", "bar");
    img.setMetadata("hello", "world");
    if (img.getMetadata("foo") != "bar" || img.getMetadata("hello") != "world")
    {
      std::cerr << "setMetadata() or getMetadata() failed" << std::endl;
      return EXIT_FAILURE;
    }

    const std::vector<std::string> keys = img.allMetadata();
    if (std::set<std::string>(keys.begin(), keys.end()) !=
      std::set<std::string>({ "foo", "hello" }))
    {
      std::cerr << "allMetadata() failed" << std::endl;
      return EXIT_FAILURE;
    }

    try
    {
      img.getMetadata("baz"); // expected to throw
      std::cerr << "getMetadata() failed to throw" << std::endl;
      return EXIT_FAILURE;
    }
    catch (std::out_of_range& e)
    {
      /* expected, key doesn't exist */
    }

    try
    {
      img.setMetadata("foo", ""); // empty value, should remove key
      img.getMetadata("foo");     // expected to throw
      std::cerr << "setMetadata() with empty value failed" << std::endl;
      return EXIT_FAILURE;
    }
    catch (std::out_of_range& e)
    {
      /* expected, key has been removed */
    }

    if (img.allMetadata() != std::vector<std::string>({ "hello" }))
    {
      std::cerr << "allMetadata() failed" << std::endl;
      return EXIT_FAILURE;
    }

    img.setMetadata("foo", ""); // make sure removing twice is ok
  }

  {
    f3d::image img1(4, 2, 3);
    img1.setMetadata("foo", "bar");
    img1.setMetadata("hello", "world");
    img1.save(tmpDir + "/metadata.png");

    f3d::image img2(tmpDir + "/metadata.png");
    if (img2.getMetadata("foo") != "bar" || img2.getMetadata("hello") != "world")
    {
      std::cerr << "saving or loading file metadata failed" << std::endl;
      return EXIT_FAILURE;
    }
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
    if (img2.getMetadata("foo") != "bar" || img2.getMetadata("hello") != "world")
    {
      std::cerr << "saving or loading buffer metadata failed" << std::endl;
      return EXIT_FAILURE;
    }
  }

  // TODO compare code path coverage
  return EXIT_SUCCESS;
}
