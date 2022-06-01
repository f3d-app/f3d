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
  image();
  ~image();
  image(const image& img);
  image& operator=(const image& img);
  image(image&& img);
  image& operator=(image&& img);
  //@}

  //@{
  /**
   * Comparison operators, uses image::compare
   */
  bool operator==(const image& reference) const;
  bool operator!=(const image& reference) const;
  //@}

  //@{
  /**
   * Set/Get image resolution
   */
  unsigned int getWidth() const;
  unsigned int getHeight() const;
  image& setResolution(unsigned int width, unsigned int height);
  //@}

  //@{
  /**
   * Set/Get image channel count
   */
  unsigned int getChannelCount() const;
  image& setChannelCount(unsigned int dim);
  //@}

  //@{
  /**
   * Set/Get image buffer data
   * Its size is expected to be width * height * channelCount
   *
   */
  image& setData(unsigned char* buffer);
  unsigned char* getData() const;
  //@}

  /**
   * Compare current image to a reference using the provided threshold.
   * If the comparison fails, output the resulting diff and error and return false,
   * return true otherwise.
   * Can throw an image::exception if images are not sane.
   */
  bool compare(const image& reference, double threshold, image& diff, double& error) const;

  /**
   * Save an image to a file in .png format
   * Can throw an image::exception if images is not sane.
   */
  image& save(const std::string& path);

private:
  class internals;
  internals* Internals;
};
}

#endif
