
#ifndef f3d_image_h
#define f3d_image_h

#include "exception.h"
#include "export.h"

#include <string>

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
   * Create an image from file, the following formats are supported:
   * PNG, PNM, TIFF, BMP, HDR, JPEG, GESigna, MetaImage, TGA.
   * Throw an image::read_exception in case of failure.
   */
  explicit image(const std::string& path);

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

  ///@{ @name Resolution
  /**
   * Set/Get image resolution.
   */
  unsigned int getWidth() const;
  unsigned int getHeight() const;
  image& setResolution(unsigned int width, unsigned int height);
  ///@}

  ///@{ @name Channel Count
  /**
   * Set/Get image channel count.
   */
  unsigned int getChannelCount() const;
  image& setChannelCount(unsigned int dim);
  ///@}

  ///@{ @name Buffer Data
  /**
   * Set/Get image buffer data.
   * Its size is expected to be `width * height * channelCount`.
   */
  image& setData(unsigned char* buffer);
  unsigned char* getData() const;
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
   * Save an image to a file in .png format.
   * Throw an image::write_exception if image cannot be written.
   */
  void save(const std::string& path) const;

  /**
   * An exception that can be thrown by the image when there.
   * is an error on write.
   */
  struct write_exception : public exception
  {
    write_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the image.
   * when there is an error on read.
   */
  struct read_exception : public exception
  {
    read_exception(const std::string& what = "");
  };

private:
  class internals;
  internals* Internals;
};
}

#endif
