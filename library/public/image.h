#ifndef f3d_image_h
#define f3d_image_h

#include "exception.h"
#include "export.h"

#include <filesystem>
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
   * Read provided file path (used as is) into a new image instance, the following formats are
   * supported: PNG, PNM, TIFF, BMP, HDR, JPEG, GESigna, MetaImage, TGA. EXR files are also
   * supported if the associated module is built. The complete list can be retrieve at runtime by
   * calling `getSupportedFormats()`. Throw an `image::read_exception` in case of failure.
   */
  explicit image(const std::filesystem::path& filePath);

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
   * Comparison operators, uses image::compare with a threshold of 1e-14.
   */
  [[nodiscard]] bool operator==(const image& reference) const;
  [[nodiscard]] bool operator!=(const image& reference) const;
  ///@}

  /**
   * Read one specific pixel and return all channel normalized values.
   * If the channel type is BYTE or SHORT, the values are normalized to [0, 1] range.
   * \warning Because of the normalization, this function can be slow, prefer getContent when
   * reading several pixels and normalization is not needed.
   */
  [[nodiscard]] std::vector<double> getNormalizedPixel(const std::pair<int, int>& xy) const;

  /**
   * Get the list of supported image format extensions when opening a file.
   */
  [[nodiscard]] static std::vector<std::string> getSupportedFormats();

  ///@{ @name Resolution
  /**
   * Set/Get image resolution.
   *
   * \deprecated { setResolution is deprecated, use the appropriate constructor }
   */
  [[nodiscard]] unsigned int getWidth() const;
  [[nodiscard]] unsigned int getHeight() const;
  ///@}

  ///@{ @name Channel Count
  /**
   * Set/Get image channel count.
   *
   * \deprecated { setChannelCount is deprecated, use the appropriate constructor }
   */
  [[nodiscard]] unsigned int getChannelCount() const;
  ///@}

  /**
   * Get image channel type.
   * throw an `image::read_exception` if the type is unknown.
   */
  [[nodiscard]] ChannelType getChannelType() const;

  /**
   * Get image channel type size in bytes.
   */
  [[nodiscard]] unsigned int getChannelTypeSize() const;

  ///@{ @name Buffer Data
  /**
   * Set/Get image buffer data.
   * Its size is expected to be `width * height * channelCount * typeSize`.
   *
   * \deprecated { setData and getData are deprecated, use setContent and getContent instead }
   */
  image& setContent(void* buffer);
  [[nodiscard]] void* getContent() const;
  ///@}

  /**
   * Compare current image to a reference.
   * The error is minimum between Minkownski and Wasserstein distance
   * on a SSIM computation, as specified in VTK.
   * Please note, due to possible arithmetic imprecision in the SSIM computation
   * a non-zero value can be returned with identical images.
   * Depending on the VTK version, another comparison algorithm may be used.
   * Error value meaning is described below:
   * 1e-14: Pixel perfect comparison.
   * 0.04: Visually indistinguishable.
   * 0.1: Small visible difference.
   * 0.5: Comparable images.
   * 1.0: Different type, size or number of components
   */
  double compare(const image& reference) const;

  /**
   * Save an image to the provided file path, used as is, in the specified format.
   * Default format is PNG if not specified.
   * PNG: Supports channel type BYTE and SHORT with channel count of 1 to 4
   * JPG: Supports channel type BYTE with channel count of 1 or 3
   * TIF: Supports channel type BYTE, SHORT and FLOAT with channel count of 1 to 4
   * BMP: Supports channel type BYTE with channel count of 1 to 4
   * Throw an `image::write_exception` if the format is incompatible with with image channel type or
   * channel count
   */
  const image& save(
    const std::filesystem::path& filePath, SaveFormat format = SaveFormat::PNG) const;

  /**
   * Save an image to a memory buffer in the specified format and returns it.
   * Default format is PNG if not specified.
   * PNG: Supports channel type BYTE and SHORT with channel count of 1 to 4
   * JPG: Supports channel type BYTE with channel count of 1 or 3
   * BMP: Supports channel type BYTE with channel count of 1 to 4
   * TIF format is not supported yet.
   * Throw an `image::write_exception` if the type is TIF or
   * if the format is incompatible with with image channel type or channel count.
   */
  [[nodiscard]] std::vector<unsigned char> saveBuffer(SaveFormat format = SaveFormat::PNG) const;

  /**
   * Convert to colored text using ANSI escape sequences for printing in a terminal.
   * Block and half-block characters are used to represent two pixels per character (vertically)
   * meaning that each text line represent two rows of pixels.
   * Byte encoded RGB and RGBA images are supported (alpha channel values are binarized).
   * Assumes terminal support for:
   * - unicode block characters (`U+2580`, `U+2584`, `U+2588`)
   * - SGR escape codes (`ESC[0m`, `ESC[49m`)
   * - 24-bit escape codes (`ESC[38;2;{r};{g};{b}m`, `ESC[48;2;{r};{g};{b}m`)
   * Throw a `image::write_exception` if the type is not byte RGB or RGBA.
   */
  const image& toTerminalText(std::ostream& stream) const;

  /**
   * Convert to colored text using ANSI escape sequences for printing in a terminal.
   * See `toTerminalText(std::ostream& stream)`.
   * Throw a `image::write_exception` if the type is not byte RGB or RGBA.
   */
  [[nodiscard]] std::string toTerminalText() const;

  /**
   * Set the value for a metadata key. Setting an empty value (`""`) removes the key.
   */
  f3d::image& setMetadata(std::string key, std::string value);

  /**
   * Get the value for a metadata key.
   * Throw a `image::read_exception` if key does not exist.
   */
  [[nodiscard]] std::string getMetadata(const std::string& key) const;

  /**
   * List all the metadata keys which have a value set.
   */
  [[nodiscard]] std::vector<std::string> allMetadata() const;

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

  /**
   * An exception that can be thrown by the image.
   * when there is an error related to metadata
   */
  struct metadata_exception : public exception
  {
    explicit metadata_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;
};
}

#endif
