#ifndef f3d_image_h
#define f3d_image_h

#include "exception.h"
#include "export.h"

#include <string>
#include <vector>

namespace f3d
{
/**
 * @class   image
 * @brief   Class used to represent an image
 *
 * A class to represent an image, which can be created from a file, a buffer and provided by a
 * window. It provides tools to save and compare images.
 */
class F3D_EXPORT image
{
public:
  /**
   * Enumeration of supported export formats
   * =======================================
   * PNG: Supports channel size up to 2 bytes
   * JPG: Supports channel size of 1 byte
   * TIF: Supports channel size up to 4 bytes
   * BMP: Supports channel size of 1 byte
   */
  enum class SaveFormat : unsigned char
  {
    PNG,
    JPG,
    TIF,
    BMP
  };

  /**
   * Enumeration of supported channel types
   * ======================================
   * BYTE: 8-bit integer in range [0,255]
   * SHORT: 16-bit integer in range [0,65535]
   * FLOAT: 32-bit floating point in range [-inf,+inf]
   */
  enum class ChannelType : unsigned char
  {
    BYTE,
    SHORT,
    FLOAT
  };

  /**
   * Create an image from file, the following formats are supported:
   * PNG, PNM, TIFF, BMP, HDR, JPEG, GESigna, MetaImage, TGA.
   * EXR files are also supported if the associated module is built.
   * The complete list can be retrieve at runtime by calling getSupportedFormats.
   * Throw an image::read_exception in case of failure.
   */
  explicit image(const std::string& path);

  /**
   * Create an image from a given width, height, and channel count.
   * A channel type can also be given. Default is BYTE.
   */
  image(unsigned int width, unsigned int height, unsigned int channelCount,
    ChannelType type = ChannelType::BYTE);

  ///@{ @name Constructors
  /**
   * Default/copy/move constructors/operators.
   */
  image();
  ~image();
  image(const image& img);
  image& operator=(const image& img) noexcept;
  image(image&& img) noexcept;
  image& operator=(image&& img) noexcept;
  ///@}

  ///@{ @name Operators
  /**
   * Comparison operators, uses image::compare with a threshold of 0.
   */
  bool operator==(const image& reference) const;
  bool operator!=(const image& reference) const;
  ///@}

  /**
   * Get the list of supported image format when opening a file.
   */
  static std::vector<std::string> getSupportedFormats();

  ///@{ @name Resolution
  /**
   * Set/Get image resolution.
   *
   * \deprecated { setResolution is deprecated, use the appropriate constructor }
   */
  unsigned int getWidth() const;
  unsigned int getHeight() const;
#ifndef F3D_NO_DEPRECATED
  F3D_DEPRECATED image& setResolution(unsigned int width, unsigned int height);
#endif
  ///@}

  ///@{ @name Channel Count
  /**
   * Set/Get image channel count.
   *
   * \deprecated { setChannelCount is deprecated, use the appropriate constructor }
   */
  unsigned int getChannelCount() const;
#ifndef F3D_NO_DEPRECATED
  F3D_DEPRECATED image& setChannelCount(unsigned int dim);
#endif
  ///@}

  /**
   * Get image channel type.
   * throw an image::read_exception if the type is unknown.
   */
  ChannelType getChannelType() const;

  /**
   * Get image channel type size in bytes.
   */
  unsigned int getChannelTypeSize() const;

  ///@{ @name Buffer Data
  /**
   * Set/Get image buffer data.
   * Its size is expected to be `width * height * channelCount * typeSize`.
   *
   * \deprecated { setData and getData are deprecated, use setContent and getContent instead }
   */
  image& setContent(void* buffer);
  void* getContent() const;
#ifndef F3D_NO_DEPRECATED
  F3D_DEPRECATED image& setData(unsigned char* buffer);
  F3D_DEPRECATED unsigned char* getData() const;
#endif
  ///@}

  /**
   * Compare current image to a reference using the provided threshold.
   * If the comparison fails, ie. error is higher than the threshold,
   * this outputs the resulting diff and error and return false,
   * return true otherwise.
   * The error is based on the pixel value and accumulated over neighbors pixels.
   * 0: Pixel perfect comparison.
   * 50: Visually indistinguishable.
   * 100: Small visible difference.
   * 300: Comparable images.
   */
  bool compare(const image& reference, double threshold, image& diff, double& error) const;

  /**
   * Save an image to a file in the specified format.
   * Default format is PNG if not specified.
   */
  void save(const std::string& path, SaveFormat format = SaveFormat::PNG) const;

  /**
   * Save an image to a memory buffer in the specified format.
   * Default format is PNG if not specified.
   * TIF format is not supported yet.
   * Throw an image::write_exception if the type is TIF.
   */
  std::vector<unsigned char> saveBuffer(SaveFormat format = SaveFormat::PNG) const;

  /**
   * Convert to colored text using ANSI escape sequences for printing in a terminal.
   * Block and half-block characters are used to represent two pixels per character (vertically)
   * meaning that each text line represent two rows of pixels.
   * Byte encoded RGB and RGBA images are supported (alpha channel values are binarized).
   * Assumes terminal support for:
   * - unicode block characters (`U+2580`, `U+2584`, `U+2588`)
   * - SGR escape codes (`ESC[0m`, `ESC[49m`)
   * - 24-bit escape codes (`ESC[38;2;{r};{g};{b}m`, `ESC[48;2;{r};{g};{b}m`)
   * Throw an exception if the type is not byte RGB or RGBA.
   */
  const f3d::image& toTerminalText(std::ostream& stream) const;

  /**
   * Convert to colored text using ANSI escape sequences for printing in a terminal.
   * See `toTerminalText(std::ostream& stream)`.
   * Throw an exception if the type is not byte RGB or RGBA.
   */
  std::string toTerminalText() const;

  /**
   * An exception that can be thrown by the image when there.
   * is an error on write.
   */
  struct write_exception : public exception
  {
    explicit write_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the image.
   * when there is an error on read.
   */
  struct read_exception : public exception
  {
    explicit read_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;
};
}

#endif
