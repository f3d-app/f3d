/**
 * @class   image
 * @brief   Class used to represent an image
 *
 * A class to represent an image, which can be created from a file, a buffer and provided by a
 * window. It provides tools to save and compare images.
 */

#ifndef f3d_image_h
#define f3d_image_h

#include "export.h"

#include <stdexcept>
#include <string>

namespace f3d
{
class F3D_EXPORT image
{
public:
  class exception : public std::runtime_error
  {
  public:
    exception(const std::string& what = "")
      : std::runtime_error(what)
    {
    }
  };

  /**
   * Create an image from file, the following formats are supported:
   * PNG, PNM, TIFF, BMP, HDR, JPEG, GESigna, MetaImage, TGA.
   * Can throw an image::exception in case of failure.
   */
  explicit image(const std::string& path);

  //@{
  /**
   * Default/copy/move constructors/operators
   */
  image() noexcept;
  ~image() noexcept;
  image(const image& img) noexcept;
  image& operator=(const image& img) noexcept;
  image(image&& img) noexcept;
  image& operator=(image&& img) noexcept;
  //@}

  //@{
  /**
   * Comparison operators, uses image::compare with a threshold of 0
   */
  bool operator==(const image& reference) const;
  bool operator!=(const image& reference) const;
  //@}

  //@{
  /**
   * Set/Get image resolution
   */
  unsigned int getWidth() const noexcept;
  unsigned int getHeight() const noexcept;
  image& setResolution(unsigned int width, unsigned int height) noexcept;
  //@}

  //@{
  /**
   * Set/Get image channel count
   */
  unsigned int getChannelCount() const noexcept;
  image& setChannelCount(unsigned int dim) noexcept;
  //@}

  //@{
  /**
   * Set/Get image buffer data
   * Its size is expected to be width * height * channelCount
   *
   */
  image& setData(unsigned char* buffer) noexcept;
  unsigned char* getData() const noexcept;
  //@}

  /**
   * Compare current image to a reference using the provided threshold.
   * If the comparison fails, ie. error is higher than the threshold,
   * this outputs the resulting diff and error and return false,
   * return true otherwise.
   * The error is based on the pixel value and accumulated over neighbors pixels.
   * 0: Pixel perfect comparison
   * 50: Visually indistinguishable
   * 100: Small visible difference
   * 300: Comparable images
   * Can throw an image::exception if images are not sane.
   */
  bool compare(const image& reference, double threshold, image& diff, double& error) const;

  /**
   * Save an image to a file in .png format
   * Can throw an image::exception if image is not sane.
   */
  void save(const std::string& path) const;

private:
  class internals;
  internals* Internals;
};
}

#endif
